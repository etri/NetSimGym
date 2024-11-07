from multiprocessing import allow_connection_pickling
import socket
import time
from NetSimGym import NetSimProto_pb2 as pb
from google.protobuf.any_pb2 import Any
import gymnasium as gym
from gymnasium import spaces
from NetSimGym import commandlist
import numpy as np
from typing import Optional, Tuple

#connect NetSim and Python
class NetSimSocketBridge:
    def __init__(self, port =8999):
        self.host_ip = '127.0.0.1'
        self.port = port
        self.init_device_name = 'UE_1'
        self.socket = None 

    def connect_to_netsim(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print("\n")
        print("                               Waiting at port {} for the NetSim to be available...".format(self.port))
        while True:
            try:
                self.socket.connect((self.host_ip, self.port))
                break  
            except socket.error:
                time.sleep(0.5)  
        print("                               Connection established to NetSim.")
        print("\n")
        time.sleep(0.1)
        name = self.init_device_name + '\0'
        self.socket.send(name.encode())

    def disconnect_from_netsim(self):
        if self.socket:
            self.socket.close()
            time.sleep(0.1)

    def create_obs_space(self,obs_space_list,commandtypelist):
        obs_space = spaces.Dict({})
        for i in range(len(obs_space_list)) :
            obs_space.spaces[commandtypelist[i]]= spaces.Box(low= obs_space_list[i][0], high= obs_space_list[i][1], 
                                                             shape=(obs_space_list[i][2],), dtype=np.float64,)
        return obs_space

    def create_act_space(self,act_space_list):        
        act_space = spaces.Dict({})
        for i in range(len(act_space_list)) :
            act_space.spaces[f'action {i}']= spaces.Box(low= act_space_list[i][0], high= act_space_list[i][1], 
                                                        shape=(act_space_list[i][2],), dtype=np.float64,)
        return act_space

    def convert_command(self, is_action, command_list):
        command_name = []
        for i in range(len(command_list)) :
            if is_action:
                command_name.append(commandlist.ActionCommandStrToNum(command_list[i]))
            else:
                command_name.append(commandlist.CommandStrToNum(command_list[i]))
        return command_name
    
    def count_command(self, device_range_list):
        CommandNum = []
        for device_range in device_range_list:
            commandnum = device_range[1] - device_range[0]
            CommandNum.append(commandnum)
        return CommandNum

    def send_msg(self, PythonMsg):
        serialized_PythonMsg = PythonMsg.SerializeToString()
        self.socket.send(serialized_PythonMsg)

    def recv_msg(self):
        serialized_NetSimMsg = self.socket.recv(4096)
        NetSimMsg = pb.NetSimMsg()
        NetSimMsg.ParseFromString(serialized_NetSimMsg)
        return NetSimMsg

    def create_init_msg(self, timestep, totaltime, command_name, device_list, device_range_list):
        init_list=[]
        for i in range(len(command_name)):
            init_msg = pb.InitMsg()
            init_msg.time_step = timestep
            init_msg.total_time = totaltime
            init_msg.command_name.append(command_name[i]) 
            for j in range(device_range_list[i][0], device_range_list[i][1]+1):
                init_msg.device_name.append(f"{device_list[i]}_{j}")            
            init_list.append(init_msg)
        return init_list

    def create_act_msg(self, action_command_list, ActionValue, isdone):
        Action_msg=[]
        for i in range(len(action_command_list)) :
            action_msg = pb.ActMsg()
            action_msg.command_name = action_command_list[i]
            action_msg.doneinfo = isdone
            for j in range(len(ActionValue[i])):
                action_msg.values.extend([ActionValue[i][j]])
            Action_msg.append(action_msg)
        return Action_msg

    def wrap_message(self, message):
        wrap_to_any=[]
        for i in range(len(message)) :
            any_msg = Any()
            any_msg.Pack(message[i])
            wrap_to_any.append(any_msg)
        return wrap_to_any

    def create_outer_msg(self, command_kind, any_msg):
        outer_message = pb.PythonMsg()
        outer_message.is_init = command_kind
        for i in range(len(any_msg)) :
            outer_message.details.append(any_msg[i])
        return outer_message
    
    def send_init_msg(self, timestep, totaltime, command_name, device_list, device_range_list):
        init_msg = self.create_init_msg(timestep, totaltime, command_name, device_list, device_range_list)
        any_msg = self.wrap_message(init_msg)
        python_msg = self.create_outer_msg(False, any_msg)
        self.send_msg(python_msg)

    def send_act_msg(self, action_command_list, ActionValue, isdone):
        act_msg = self.create_act_msg(action_command_list, ActionValue, isdone)
        any_msg = self.wrap_message(act_msg)
        python_msg = self.create_outer_msg(True, any_msg)
        self.send_msg(python_msg)

    def get_obs_msg(self, isreset, CommandNum):
        netsim_msg = self.recv_msg()
        obs_msg = {}
        reward_dict = []
        terminated = netsim_msg.is_done
        
        for command_count in CommandNum:
            obs_index = 0
            for j in range(command_count):
                if obs_index < len(netsim_msg.details):
                    any_msg = netsim_msg.details[obs_index]
                    if any_msg.type_url == "type.googleapis.com/ObsMsg":
                        obs = pb.ObsMsg()
                        unpack_obsmsg = any_msg.Unpack(obs)
                        if unpack_obsmsg:
                            if obs.is_reward:
                                reward_dict.append({obs.key_name: np.array(list(obs.values))})  
                            else:
                                obs_msg[obs.key_name] = np.array(list(obs.values))
                    obs_index += 1            
        for item in reward_dict:
            if len(item) == 1 and len(list(item.values())[0]) == 1:
                reward = list(item.values())[0][0]
            else:
                reward = reward_dict

                
        info = {"information": netsim_msg.more_info}
        if isreset:
            return obs_msg, info
        else:
            return obs_msg, reward,terminated,info

#gym api
class NetSimEnv(gym.Env):
    def __init__(self, port = 8999, timestep=1, totaltime=50, 
                 State_list=[], State_device_list=[], State_device_range_list=[], Action_list=[], 
                 Reward_list=[], Reward_device_list=[], Reward_device_range_list=[], 
                 obs_space_list=[] , act_space_list=[]):
        self.port = port
        self.timestep = timestep
        self.totaltime = totaltime
        self.currentstep = 0
        self.totalstep = totaltime/timestep

        self.bridge = NetSimSocketBridge(port)
        self.observation_space = self.bridge.create_obs_space(obs_space_list,State_list)  
        self.action_space = self.bridge.create_act_space(act_space_list)

        command_list = State_list + Reward_list
        self.device_list = State_device_list + Reward_device_list
        self.device_range_list = State_device_range_list + Reward_device_range_list

        self.command_name = self.bridge.convert_command(False, command_list)
        self.action_name = self.bridge.convert_command(True, Action_list)

        self.CommandNum = self.bridge.count_command(self.device_range_list)

    def reset(self, seed: Optional[int] = None, options: Optional[dict] = None):
        if seed is not None:
            np.random.seed(seed)

        self.currentstep = 1
        isreset = True
        self.bridge.connect_to_netsim()
        self.bridge.send_init_msg(self.timestep, self.totaltime, self.command_name, self.device_list, self.device_range_list)
        obs_msg, info = self.bridge.get_obs_msg(isreset, self.CommandNum) 
        return obs_msg, info
       
    def step(self, ActionValue):
        self.currentstep += 1
        isreset = False
        truncated = self.currentstep >= self.totalstep
        self.bridge.send_act_msg(self.action_name, ActionValue, truncated)

        try:
            obs_msg, reward, terminated, info = self.bridge.get_obs_msg(isreset, self.CommandNum)
            return obs_msg, reward, terminated, truncated, info
        
        except Exception as e:
            processed_details = {}
            reward = []
            terminated = 0
            info = []
            return processed_details, reward, terminated, truncated, info
    
    def render(self, mode='human'):
        return
    
    def close(self):
        self.bridge.disconnect_from_netsim
    
    def get_random_action(self):
        act = self.action_space.sample()
        return act
    
