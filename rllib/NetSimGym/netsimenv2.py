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
import logging
import os

#connect NetSim and Python
class NetSimSocketBridge:
    def __init__(self, port =8999):
        self.host_ip = '127.0.0.1'
        self.port = port
        self.init_device_name = 'UE_1'
        self.socket = None

    def connect_to_netsim(self):
        while True:
            try:
                self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                print("                               Waiting at port {} for the NetSim to be available...".format(self.port))
                try:
                    self.socket.connect((self.host_ip, self.port))
                except socket.error as conn_error:
                    time.sleep(0.5)
                    continue

                time.sleep(0.1)

                try:
                    handshake_msg = self.socket.recv(1024).decode()
                except socket.error as recv_error:
                    self.socket.close()
                    time.sleep(0.5)
                    continue

                if handshake_msg == "FIRST_CLIENT":
                    print("                               Connection established to NetSim.", "port: ", self.port, "time: ", time.ctime())
                    name = self.init_device_name + '\0'
                    try:
                        self.socket.send(name.encode())
                    except socket.error as send_error:
                        self.socket.close()
                        time.sleep(0.5)
                        continue
                    break
                else:
                    print("                               NetSim not finished. Retrying connection...")
                    self.socket.close()
                    time.sleep(0.5)
            except Exception as e:
                try:
                    self.socket.close()
                except:
                    pass
                time.sleep(0.5)

    def disconnect_from_netsim(self):
        if self.socket:
            self.socket.close()
            time.sleep(0.1)

    def obs_to_reward(self, reward_div, next_serving_load, next_neighbor_load, serving_load, neighbor_load, all_loads):
        reward = 0
        change_in_serving = next_serving_load - serving_load
        change_in_neighbors = next_neighbor_load - neighbor_load

        if change_in_serving <= reward_div[0]:
            reward += 2
        elif change_in_serving <= reward_div[1]:
            reward += 1
        elif change_in_serving <= reward_div[2]:
            reward += 0
        elif change_in_serving <= reward_div[3]:
            reward -= 1
        else:
            reward -= 2

        if change_in_neighbors >= reward_div[3]:
            additional_penalty = -1
        else:
            additional_penalty = 0

        reward += additional_penalty

        # Reflect balance between cell7 and overall average load
        avg_load = sum(all_loads) / len(all_loads)
        load_difference = abs(next_serving_load - avg_load)
        balance_reward = -load_difference
        reward += balance_reward

        return reward

    def convert_command(self, is_action, command_list):
        command_name = []
        for i in range(len(command_list)):
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
        init_list = []
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
        Action_msg = []
        for i in range(len(action_name)):
            action_msg = pb.ActMsg()
            action_msg.command_name = action_name[i]
            action_msg.doneinfo = isdone
            for value in ActionValue[action_list[i]]:
                action_msg.values.append(value)
            Action_msg.append(action_msg)
        return Action_msg

    def wrap_message(self, message):
        wrap_to_any = []
        for i in range(len(message)):
            any_msg = Any()
            any_msg.Pack(message[i])
            wrap_to_any.append(any_msg)
        return wrap_to_any

    def create_outer_msg(self, command_kind, any_msg):
        outer_message = pb.PythonMsg()
        outer_message.is_init = command_kind
        for i in range(len(any_msg)):
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
            return obs_msg, reward, terminated, info

#gym api
class NetSimEnv2(gym.Env):
    def __init__(self, env_config: Optional[dict] = None):
        self.env_config = env_config if env_config is not None else {}
        self.port = self.env_config.get('port', 8999)
        self.timestep = self.env_config.get('timestep', 0.5)
        self.totaltime = self.env_config.get('totaltime', 15)
        self.currentstep = 0
        totalstep_ratio = self.totaltime / self.timestep
        if np.isclose(np.mod(self.totaltime, self.timestep), 0) or np.isclose(np.mod(self.totaltime, self.timestep), self.timestep):
            self.totalstep = int(totalstep_ratio) - 1
        else:
            self.totalstep = int(np.floor(totalstep_ratio))

        self.State_list = self.env_config.get('State_list', ["CellLoad_ex2"])
        self.State_device_list = self.env_config.get('State_device_list', ["gNB"])
        self.State_device_range_list = self.env_config.get('State_device_range_list', [[9, 15]])
        self.Action_list = self.env_config.get('Action_list', ["HOM"])
        self.Reward_list = self.env_config.get('Reward_list', ["Dummy"])
        self.Reward_device_list = self.env_config.get('Reward_device_list', ["UE"])
        self.Reward_device_range_list = self.env_config.get('Reward_device_range_list', [[16, 16]])

        # Continuous observation space: 7 cell loads (CellLoad_ex2)
        self.obs_space_list = [
            [np.float32(low), np.float32(high), int(shape)]
            for low, high, shape in self.env_config.get('obs_space_list', [[0, 2, 7]])
        ]
        # Continuous action space: delta HOM for cell 7 (same concept as discrete adjustments)
        self.max_delta = np.float32(self.env_config.get('max_delta', 0.4))

        # Reward thresholds for obs_to_reward function
        self.reward_div = self.env_config.get('reward_div', [-0.5, -0.2, 0, 0.2, 0.5])

        self.bridge = NetSimSocketBridge(self.port)

        # Continuous (Box) observation space
        obs_low  = np.full((self.obs_space_list[0][2],), self.obs_space_list[0][0], dtype=np.float32)
        obs_high = np.full((self.obs_space_list[0][2],), self.obs_space_list[0][1], dtype=np.float32)
        self.observation_space = Box(low=obs_low, high=obs_high, dtype=np.float32)

        # Continuous (Box) action space: single delta value in [-max_delta, max_delta]
        self.action_space = Box(low=-self.max_delta, high=self.max_delta, shape=(1,), dtype=np.float32)

        command_list = self.State_list + self.Reward_list
        self.device_list = self.State_device_list + self.Reward_device_list
        self.device_range_list = self.State_device_range_list + self.Reward_device_range_list

        self.command_name = self.bridge.convert_command(False, command_list)
        self.action_name = self.bridge.convert_command(True, self.Action_list)

        self.CommandNum = self.bridge.count_command(self.device_range_list)
        self.iteration = 0

        # Setup CSV logging for episode statistics
        home_dir = os.path.expanduser("~")
        path_logs = os.path.join(home_dir, "Desktop", "work2025final", "netsimenv2_logs")
        os.makedirs(path_logs, exist_ok=True)

        self.log_filename = os.path.join(path_logs, "episode_statistics.csv")
        self.step_log_filename = os.path.join(path_logs, "step_statistics.csv")

        if not os.path.exists(self.log_filename):
            with open(self.log_filename, 'w') as f:
                f.write("Episode,Cell7_Average_Load,Total_Average_Load\n")

        if not os.path.exists(self.step_log_filename):
            with open(self.step_log_filename, 'w') as f:
                f.write("Episode,Step,Cell1_Load,Cell2_Load,Cell3_Load,Cell4_Load,Cell5_Load,Cell6_Load,Cell7_Load,Avg_Load\n")

        self.episode_cell_loads = []

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

        load_values = obs_msg["CellLoad_ex2"]
        self.serving_load = load_values[6]
        self.neighbor_load = sum(load_values[0:6]) / 6

        # Continuous state: raw cell load values (shape=(7,))
        state = np.array(load_values[0:7], dtype=np.float32)

        self.episode_cell_loads = []
        self.current_HOM = [3.0 for _ in range(7)]
        print("port: ", self.port, "reset - state:", state)

        return state, info

    def step(self, action):
        self.currentstep += 1
        isreset = False
        truncated = False
        islaststep = self.currentstep >= self.totalstep

        # action: continuous delta for cell 7 HOM (same accumulation logic as discrete)
        delta_h = float(action[0])
        current_HOM_center = self.current_HOM[6] + delta_h
        current_HOM_center = max(0.0, min(6.0, current_HOM_center))
        current_HOM_rest = 6.0 - current_HOM_center
        self.current_HOM = [current_HOM_rest] * 6 + [current_HOM_center]
        ActionValue = {self.Action_list[0]: self.current_HOM}

        print(f"  [Step {self.currentstep}] Delta H: {delta_h:.3f}, HOM center: {current_HOM_center:.3f}")
        time.sleep(0.01)
        self.bridge.send_act_msg(self.Action_list, self.action_name, ActionValue, truncated)

        try:
            obs_msg, reward, terminated, info = self.bridge.get_obs_msg(isreset, self.CommandNum)
            load_values = obs_msg["CellLoad_ex2"]
            next_serving_load = load_values[6]
            next_neighbor_load = sum(load_values[0:6]) / 6

            all_loads = list(load_values[0:7])
            avg_load = sum(all_loads) / len(all_loads)

            # Reward: same obs_to_reward as netsimenv2_discrete
            reward = self.bridge.obs_to_reward(self.reward_div, next_serving_load, next_neighbor_load,
                                               self.serving_load, self.neighbor_load, all_loads)

            # Record cell loads for this step
            self.episode_cell_loads.append(all_loads)

            # Log step statistics to CSV
            with open(self.step_log_filename, 'a') as f:
                cell_loads_str = ','.join([f"{float(load):.4f}" for load in all_loads])
                f.write(f"{self.iteration},{self.currentstep},{cell_loads_str},{avg_load:.4f}\n")

            print(f"  [Step {self.currentstep}] Cell loads: {[round(float(v), 2) for v in all_loads]} | Avg load: {avg_load:.2f} | Cell7 load: {float(next_serving_load):.2f} | Reward: {reward}")

            self.serving_load = next_serving_load
            self.neighbor_load = next_neighbor_load

            # Continuous state: raw cell load values (shape=(7,))
            next_state = np.array(load_values[0:7], dtype=np.float32)

            if islaststep:
                truncated = True
                self.bridge.send_act_msg(self.Action_list, self.action_name, ActionValue, truncated)
                print("port: ", self.port, "truncated:", truncated, "time: ", time.ctime())

                if len(self.episode_cell_loads) > 0:
                    cell7_loads = [loads[6] for loads in self.episode_cell_loads]
                    cell7_avg = np.mean(cell7_loads)
                    all_episode_loads = np.array(self.episode_cell_loads)
                    total_avg = np.mean(all_episode_loads)
                    with open(self.log_filename, 'a') as f:
                        f.write(f"{self.iteration},{cell7_avg:.4f},{total_avg:.4f}\n")

            return next_state, reward, terminated, truncated, info

        except Exception as e:
            print(f"Error in step: {e}")
            state = np.zeros(self.obs_space_list[0][2], dtype=np.float32)

            reward = 0
            terminated = 0
            info = {}
            truncated = True
            return state, reward, terminated, truncated, info

    def render(self, mode='human'):
        return

    def close(self):
        self.bridge.disconnect_from_netsim()

    def get_random_action(self):
        return self.action_space.sample()
