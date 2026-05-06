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
from gymnasium.spaces import Discrete, Box

#connect NetSim and Python
class NetSimSocketBridge:
    def __init__(self, port =8999):
        self.host_ip = '127.0.0.1'
        self.port = port
        self.init_device_name = 'UE_1'
        self.socket = None 

    def connect_to_netsim(self):
        # self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # print("\n")
        # print("                               socket Waiting at port {} for the NetSim to be available...".format(self.port))
        while True:
            try:
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                #print("\n")
                print("                               Waiting at port {} for the NetSim to be available...".format(self.port))
                try:
                    self.socket.connect((self.host_ip, self.port))
                    #print("connected")
                except socket.error as conn_error:
                    #print("Connection error: {}. Retrying...".format(conn_error))
                    time.sleep(0.5) 
                    continue 

                time.sleep(0.1)

                try:
                    handshake_msg = self.socket.recv(1024).decode()
                    #print("Received handshake message from NetSim: {}".format(handshake_msg))
                except socket.error as recv_error:
                    #print("Recv error: {}. Retrying connection...".format(recv_error))
                    self.socket.close()
                    time.sleep(0.5)
                    continue

                if handshake_msg == "FIRST_CLIENT":
                    print("                               Connection established to NetSim.", "port: ", self.port, "time: ", time.ctime())
                    #print("\n")
                    name = self.init_device_name + '\0'
                    try:
                        self.socket.send(name.encode())
                    except socket.error as send_error:
                        #print("Send error: {}. Retrying connection...".format(send_error))
                        self.socket.close()
                        time.sleep(0.5)
                        continue
                    break
                else:
                    print("                               NetSim not finished. Retrying connection...")
                    self.socket.close()
                    time.sleep(0.5)
            except Exception as e:
                #print("Unexpected error: {}. Retrying connection...".format(e))
                try:
                    self.socket.close()
                except:
                    pass
                time.sleep(0.5)
        
    def disconnect_from_netsim(self):
        if self.socket:
            self.socket.close()
            time.sleep(0.1)

    def create_obs_space(self,obs_space_list,state_list):
        obs_space = spaces.Dict({})
        for i in range(len(state_list)) :
            obs_space.spaces[state_list[i]]= spaces.Box(low= obs_space_list[i][0], high= obs_space_list[i][1], 
                                                             shape=(obs_space_list[i][2],), dtype=np.float32,)
        return obs_space

    def create_act_space(self,act_space_list, action_list):        # if many actions
        act_space = spaces.Dict({})
        for i in range(len(action_list)) :
            act_space.spaces[action_list[i]]= spaces.Box(low= act_space_list[i][0], high= act_space_list[i][1], 
                                                        shape=(act_space_list[i][2],), dtype=np.float32,)
        return act_space

    # def create_obs_space(self,obs_space_list,state_list):
    #     obs_space = spaces.Dict({})
    #     for i in range(len(state_list)) :
    #         low = np.full((obs_space_list[i][2],), obs_space_list[i][0], dtype=np.float32)
    #         high = np.full((obs_space_list[i][2],), obs_space_list[i][1], dtype=np.float32)
    #         obs_space.spaces[state_list[i]]= spaces.Box(low= low, high= high, dtype=np.float32,)
    #     return obs_space

    # def create_act_space(self,act_space_list, action_list):        # if many actions
    #     act_space = spaces.Dict({})
    #     for i in range(len(action_list)) :
    #         low = np.full((act_space_list[i][2],), act_space_list[i][0], dtype=np.float32)
    #         high = np.full((act_space_list[i][2],), act_space_list[i][1], dtype=np.float32)
    #         act_space.spaces[action_list[i]]= spaces.Box(low= low, high=high, dtype=np.float32,)
    #     return act_space

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
            commandnum = device_range[1] - device_range[0] # check
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

    def create_act_msg(self, action_list, action_name, ActionValue, isdone):
        Action_msg=[]
        for i in range(len(action_name)) :
            action_msg = pb.ActMsg()
            action_msg.command_name = action_name[i]
            action_msg.doneinfo = isdone
            #for j in range(len(ActionValue[i])):
            for value in ActionValue[action_list[i]]:
                action_msg.values.append(value)
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

    def send_act_msg(self, action_list, action_name, ActionValue, isdone):
        act_msg = self.create_act_msg(action_list, action_name, ActionValue, isdone)
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
                while obs_index < len(netsim_msg.details):
                    any_msg = netsim_msg.details[obs_index]
                    if any_msg.type_url == "type.googleapis.com/ObsMsg":
                        obs = pb.ObsMsg()
                        unpack_obsmsg = any_msg.Unpack(obs)
                        if unpack_obsmsg:
                            if obs.is_reward:
                                reward_dict.append({obs.key_name: np.array(list(obs.values), dtype=np.float32)})  
                            else:
                                obs_msg[obs.key_name] = np.array(list(obs.values), dtype=np.float32)
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
class NetSimEnv0(gym.Env):
    # def __init__(self, port = 8999, timestep=1, totaltime=50, 
    #              State_list=[], State_device_list=[], State_device_range_list=[], Action_list=[], 
    #              Reward_list=[], Reward_device_list=[], Reward_device_range_list=[], 
    #              obs_space_list=[] , act_space_list=[]):
    def __init__(self, env_config: Optional[dict] = None):
        self.env_config = env_config if env_config is not None else {}
        self.port = self.env_config.get('port', 8999)
        self.timestep = self.env_config.get('timestep', 0.5)
        self.totaltime = self.env_config.get('totaltime', 15)
        self.currentstep = 0
        totalstep_ratio = self.totaltime/self.timestep
        if np.isclose(np.mod(self.totaltime, self.timestep), 0) or np.isclose(np.mod(self.totaltime, self.timestep), self.timestep): #added 'or' condition for floating point error
            self.totalstep = int(totalstep_ratio) -1
        else: 
            self.totalstep = int(np.floor(totalstep_ratio))
        # print ("total step number is: ", self.totalstep)
        # print ("step for train batch size is total step minus one:", self.totalstep-1)
        #self.totalstep = self.totaltime/self.timestep
        self.State_list =self.env_config.get('State_list', ["RandomState"])
        self.State_device_list =self.env_config.get('State_device_list', ["UE"])
        self.State_device_range_list =self.env_config.get('State_device_range_list', [[7, 12]])
        self.Action_list =self.env_config.get('Action_list', ["RandomAction"])
        self.Reward_list =self.env_config.get('Reward_list', ["Dummy"])
        self.Reward_device_list =self.env_config.get('Reward_device_list', ["UE"]) # reward must me a single value
        self.Reward_device_range_list =self.env_config.get('Reward_device_range_list', [[12, 12]]) # need to change
        # self.obs_space_list =self.env_config.get('obs_space_list', [ [0, 5, 6] ]) 
        # self.act_space_list =self.env_config.get('act_space_list', [ [0, 5, 3] ])
        self.obs_space_list = [
                               [np.float32(low), np.float32(high), int(shape)]
                               for low, high, shape in self.env_config.get('obs_space_list', [[0, 5, 6]])
        ]
        self.act_space_list = [
            [np.float32(low), np.float32(high), int(shape)]
            for low, high, shape in self.env_config.get('act_space_list', [[0, 5, 3]])
        ]

        self.bridge = NetSimSocketBridge(self.port)
        self.observation_space = self.bridge.create_obs_space(self.obs_space_list, self.State_list)  
        self.action_space = self.bridge.create_act_space(self.act_space_list, self.Action_list)

        command_list = self.State_list + self.Reward_list
        self.device_list = self.State_device_list + self.Reward_device_list
        self.device_range_list = self.State_device_range_list + self.Reward_device_range_list

        self.command_name = self.bridge.convert_command(False, command_list)
        self.action_name = self.bridge.convert_command(True, self.Action_list)

        self.CommandNum = self.bridge.count_command(self.device_range_list)
        self.iteration = 0

    def reset(self, seed: Optional[int] = None, options: Optional[dict] = None):
        if seed is not None:
            np.random.seed(seed)
        self.iteration += 1
        print("port: ", self.port, "- iteration: ", self.iteration, "time: ", time.ctime())
        self.currentstep = 1
        isreset = True
        self.bridge.connect_to_netsim()
        self.bridge.send_init_msg(self.timestep, self.totaltime, self.command_name, self.device_list, self.device_range_list)
        obs_msg, info = self.bridge.get_obs_msg(isreset, self.CommandNum) 

        print ("port: ", self.port, "reset - obs_msg:", obs_msg)

        return obs_msg, info
       
    def step(self, ActionValue):
        self.currentstep += 1
        isreset = False
        truncated  = False
        islaststep = self.currentstep >= self.totalstep

        #print (self.currentstep, "action:", ActionValue)
        time.sleep(0.01)
        self.bridge.send_act_msg(self.Action_list, self.action_name, ActionValue, truncated)

        try:
            obs_msg, reward, terminated, info = self.bridge.get_obs_msg(isreset, self.CommandNum)
            #print ("obs_msg:", obs_msg)
            if islaststep:
                truncated = True
                print ("obs_msg:", obs_msg)
                self.bridge.send_act_msg(self.Action_list, self.action_name, ActionValue, truncated)
                # time.sleep(2.5)
                print ("port: ", self.port, "truncated:", truncated, "time: ", time.ctime())
            return obs_msg, reward, terminated, truncated, info
        
        except Exception as e:
            processed_details = {}
            print (processed_details)
            reward = []
            terminated = 0
            info = []
            truncated = True
            return processed_details, reward, terminated, truncated, info
    
    def render(self, mode='human'):
        return
    
    def close(self):
        self.bridge.disconnect_from_netsim
    
    def get_random_action(self):
        act = self.action_space.sample()
        return act
    
