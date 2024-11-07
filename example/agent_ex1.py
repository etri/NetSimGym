import gymnasium as gym
import numpy as np
import NetSimGym
import matplotlib.pyplot as plt
import logging
import pandas as pd
import os

######################################################## Function Definition ###########################################################
def sinr_to_state(percentiles, num_UE_buckets, sinr_values):
    state = 0
    for sinr in sinr_values:
        if sinr < percentiles[0]:
            state = state * num_UE_buckets + 0
        elif sinr < percentiles[1]:
            state = state * num_UE_buckets + 1
        else:
            state = state * num_UE_buckets + 2
    return state

def action_to_power_adjustments(num_eNBs, num_eNB_buckets, adjustments, action_index):
    delta_p = []
    for i in range(num_eNBs):
        delta_p.append(adjustments[action_index % num_eNB_buckets])
        action_index //= num_eNB_buckets
    return delta_p

def setting_paths():
    parent_dir = os.getcwd()
    directory_plots = "ex1_lte_plots"
    path_plots = os.path.join(parent_dir, directory_plots)
    print(os.path.isdir(path_plots))
    if(not os.path.isdir(path_plots)):
        os.mkdir(path_plots)

    directory_logs = "ex1_lte_logs"
    path_logs = os.path.join(parent_dir, directory_logs)
    print(os.path.isdir(path_logs))
    if(not os.path.isdir(path_logs)):
        os.mkdir(path_logs)
    return path_plots, path_logs

def setting_power_log(path_logs, num_eNBs):
    log_filename = os.path.join(path_logs, "eNB_powers_log.csv")
    logger = logging.getLogger('eNB_Powers_Logger')
    logger.setLevel(logging.INFO)
    eNB_powers_csv_handler = logging.FileHandler(log_filename, mode='w')
    eNB_powers_csv_handler.setFormatter(logging.Formatter('%(message)s'))
    logger.addHandler(eNB_powers_csv_handler)
    header_eNB = ','.join([f'eNB_Power_{i+1}' for i in range(num_eNBs)]) + '\n'
    eNB_powers_csv_handler.stream.write(header_eNB)
    return logger
    
def log_eNB_powers(logger, eNB_powers):
    logger.info(",".join(map(str, eNB_powers)))

def setting_reward_log(path_logs, scaling_factor):
    reward_log_filename = os.path.join(path_logs, "episode_rewards_log.csv")

    reward_logger = logging.getLogger('RewardLogger')
    reward_logger.setLevel(logging.INFO)
    reward_handler = logging.FileHandler(reward_log_filename, mode='w')
    reward_logger.addHandler(reward_handler)

    headers = ["Episode"] + [f"Reward_Scale_{i+1}" for i in range(scaling_factor)]
    reward_logger.info(",".join(headers))

    return reward_logger

def log_episode_rewards(logger, episode, rewards, steps_per_episode):
    normalized_rewards = [reward / steps_per_episode for reward in rewards]
    log_entry = [episode] + normalized_rewards
    logger.info(",".join(map(str, log_entry)))

def plot_results(total_rewards_per_episode, path_plots, path_logs, num_eNBs):
    plt.figure(figsize=(10, 6))
    for i, rewards in enumerate(total_rewards_per_episode):
        plt.plot(rewards, label=f'cluster {i+1} rewards')
    plt.legend()
    plt.title("Average rewards per episode")
    plt.xlabel("Episodes")
    plt.ylabel("Average Sum Throughput (Mbps)")
    plot_filename = os.path.join(path_plots, "Sum_Throughputs.png")
    plt.savefig(plot_filename)
    plt.show()

    log_filename = os.path.join(path_logs, "eNB_powers_log.csv")
    eNB_DF = pd.read_csv(log_filename)
    plt.figure(figsize=(10, 6))
    for i in range(num_eNBs):
        plt.plot(eNB_DF[f'eNB_Power_{i+1}'], label=f'eNB{i+1}')
    plt.title("eNB Powers vs. Time")
    plt.xlabel("Iterations")
    plt.ylabel("Power (dBm)")
    plt.legend()
    plot_filename = os.path.join(path_plots, "eNB_Powers.png")
    plt.savefig(plot_filename)
    plt.show()
########################################################################################################################################

def main():

######################################################## Scenario Setting for Gym ######################################################
    port = 8999
    timestep = 0.5
    totaltime = 15
    scaling_factor = 1 # change for each scenario
    num_eNBs = 3 * scaling_factor
    num_UEs = 6 * scaling_factor
    
    if scaling_factor == 1:
        State_list = ["SINR_ex1"] 
        State_device_list = ["UE"]
        State_device_range_list = [[7, 12]]
    elif scaling_factor == 2:
        State_list = ["SINR_ex1", "SINR_ex1"]
        State_device_list = ["UE", "UE"]
        State_device_range_list = [[7, 12], [16, 21]]    
    elif scaling_factor == 3:
        State_list = ["SINR_ex1", "SINR_ex1", "SINR_ex1"]
        State_device_list = ["UE", "UE", "UE"]
        State_device_range_list = [[7, 12], [16, 21], [25, 30]]        
    elif scaling_factor == 4:
        State_list = ["SINR_ex1", "SINR_ex1", "SINR_ex1", "SINR_ex1"]
        State_device_list = ["UE", "UE", "UE", "UE"]
        State_device_range_list = [[7, 12], [16, 21], [25, 30], [34, 39]]    
    elif scaling_factor == 5:
        State_list = ["SINR_ex1", "SINR_ex1", "SINR_ex1", "SINR_ex1", "SINR_ex1"]
        State_device_list = ["UE", "UE", "UE", "UE", "UE"]
        State_device_range_list = [[7, 12], [16, 21], [25, 30], [34, 39], [43, 48]]

    Action_list = ["Power"]
    Reward_list = ["Sum_ex1"]
    Reward_device_list = ["UE"]
    Reward_device_range_list = [[12, 12]] 
    obs_space_list = [ [-20, 20, num_UEs] ]
    act_space_list = [ [-5, 5, num_eNBs] ]

    env_id = 'NetSim-v0'
    kwargs = {
        'port': port,
        'timestep': timestep,
        'totaltime' : totaltime,
        'State_list': State_list,
        'State_device_list': State_device_list,
        'State_device_range_list': State_device_range_list,
        'Action_list': Action_list,
        'Reward_list': Reward_list,
        'Reward_device_list': Reward_device_list,
        'Reward_device_range_list': Reward_device_range_list,
        'obs_space_list': obs_space_list,
        'act_space_list': act_space_list
    }
########################################################################################################################################

######################################################## Scenario Setting for RL #######################################################
    current_episode_num = 0 
    max_episode_num = 300
    current_time = 0
    adjustments = [-3, -1, 0, 1, 3]
    percentiles = [-7, 3]
    num_UE_buckets = len(percentiles) + 1
    num_eNB_buckets = len(adjustments)
    num_states = num_UE_buckets ** 6
    num_actions = num_eNB_buckets ** 3

    states = [0] * scaling_factor
    next_states = [0] * scaling_factor
    action_indices = [0] * scaling_factor
    delta_p = [[0] * 3 for _ in range(scaling_factor)]
    current_eNB_powers = [[0] * 3 for _ in range(scaling_factor)]
    rewards = [0] * scaling_factor
    total_rewards = [0] * scaling_factor
    q_tables = [np.zeros((num_states, num_actions)) for _ in range(scaling_factor)]
    exploration = max_episode_num / 2
    epsilon = 1
    EPSILON_DECAY = 0.25 ** (1 / exploration)
    MIN_EPSILON = 0.25
    gamma = 0.9
    alpha = 0.3
    total_rewards_per_episode = [[] for _ in range(scaling_factor)]
    steps_per_episode = int(totaltime/timestep)
########################################################################################################################################
 
    path_plots, path_logs = setting_paths()
    logger = setting_power_log(path_logs, num_eNBs)
    reward_logger = setting_reward_log(path_logs, scaling_factor)

    env = gym.make(env_id, **kwargs)

    while current_episode_num < max_episode_num:
        
        current_eNB_powers = [[40] * 3 for _ in range(scaling_factor)]
        total_rewards = [0] * scaling_factor
        obs, info = env.reset()
        sinr_values = obs["SINR_ex1"]
        current_time = timestep
        for i in range(scaling_factor):
            states[i] = sinr_to_state(percentiles, num_UE_buckets, sinr_values[i:i+6])

        while True:
            print(f"{'-' * 43} {current_time:.1f} {'-' * 43}\n")   
            print("            State :")
            for key, value in obs.items():
                print(f"            - {key} : {value}")

######################################################## Action Decision ###############################################################
            ActionValue=[]
            for i in range(scaling_factor) :
                if np.random.rand() < epsilon:
                    action_indices[i] = np.random.randint(num_actions)
                else:
                    action_indices[i] = np.argmax(q_tables[i][states[i]])
                delta_p[i] = action_to_power_adjustments(3, num_eNB_buckets, adjustments, action_indices[i])
                current_eNB_powers[i] = [p + dp for p, dp in zip(current_eNB_powers[i], delta_p[i])]
                current_eNB_powers[i] = [max(27, min(46, p)) for p in current_eNB_powers[i]]
            current_eNB_powers_list = []
            for sublist in current_eNB_powers:
                current_eNB_powers_list.extend(sublist)
            determinedvalue1 = current_eNB_powers_list
            ActionValue.extend([determinedvalue1])
            print("            Action :", ActionValue )
########################################################################################################################################
            obs, reward_list, terminated, truncated, info = env.step(ActionValue)
           
            if not (terminated or truncated): 
                sinr_values = obs["SINR_ex1"]
                for i in range(scaling_factor):
                    next_states[i] = sinr_to_state(percentiles, num_UE_buckets, sinr_values[i:i+6])

                if scaling_factor == 1:
                    for i in range(scaling_factor):
                        rewards[i] = reward_list
                        print(f"            Reward : ", rewards[i])
                        total_rewards[i] += rewards[i]
                elif scaling_factor != 1:
                    for i in range(scaling_factor):
                        rewards[i] = reward_list[0]['Sum_ex1'][i]
                        print(f"            Reward {i}: ", rewards[i])
                        total_rewards[i] += rewards[i]
                print("\n")

######################################################## Q-learning #####################################################################
                for i in range(scaling_factor):
                    q_tables[i][states[i], action_indices[i]] += alpha * (rewards[i] + gamma * np.max(q_tables[i][next_states[i]]) - q_tables[i][states[i], action_indices[i]])          
                    states[i] = next_states[i]
##########################################################################################################################################

            current_time +=timestep

            if current_episode_num == max_episode_num-1:
                log_eNB_powers(logger, current_eNB_powers_list)

            if (terminated or truncated):
                current_episode_num += 1
                for i in range(scaling_factor):
                    total_rewards_per_episode[i].append(total_rewards[i] / steps_per_episode)
                
                if epsilon > MIN_EPSILON:
                    epsilon *= EPSILON_DECAY
                    epsilon = max(MIN_EPSILON, epsilon)

                print(f"\n{'*' * 37} Simulation completed {'*' * 38}")
                print(f"{'*' * 37} CurrentEpisodeNum : {current_episode_num} {'*' * 37}")
                for i in range(scaling_factor):
                    print(f"{'*' * 37} Reward: {total_rewards[i]/steps_per_episode} {'*' * 32}")
                log_episode_rewards(reward_logger, current_episode_num, total_rewards, steps_per_episode)
                break
    env.close()
    print(f"\n{'*' * 45} Done {'*' * 46}")            
    plot_results(total_rewards_per_episode, path_plots, path_logs, num_eNBs)

if __name__ == "__main__":
    main()