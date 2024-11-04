import gymnasium as gym
import numpy as np
import NetSimGym
import matplotlib.pyplot as plt
import logging
import pandas as pd
import os

######################################################## Function Definition ######################################################
def get_bucket_index(value, dividers):
    for i, div in enumerate(dividers):
        if value < div:
            return i
    return len(dividers)

def extract_obs_data(load_values, edge_values):
    serving_load = load_values[6]
    neighbor_load = sum(load_values[0:6]) / 6
    edge_ratio = edge_values[6]

    return serving_load, neighbor_load, edge_ratio

def obs_to_state(serving_div, neighbor_div, edge_div, serving_load, neighbor_load, edge_ratio):
    num_neighbor_buckets = len(neighbor_div) + 1
    num_edge_buckets = len(edge_div) + 1
    state = [0]

    serving_index = get_bucket_index(serving_load, serving_div)
    neighbor_index = get_bucket_index(neighbor_load, neighbor_div)
    edge_index = get_bucket_index(edge_ratio, edge_div)

    state = serving_index
    state = state * num_neighbor_buckets + neighbor_index
    state = state * num_edge_buckets + edge_index

    return state

def obs_to_reward(reward_div, next_serving_load, next_neighbor_load, serving_load, neighbor_load):
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

    return reward

def action_to_HOM_adjustments(adjustments, action_index):
    delta_h = adjustments[action_index]
    return delta_h

def setting_paths():
    parent_dir = os.getcwd()
    directory_plots = "ex2_plots"
    path_plots = os.path.join(parent_dir, directory_plots)
    print(os.path.isdir(path_plots))
    if(not os.path.isdir(path_plots)):
        os.mkdir(path_plots)

    directory_logs = "ex2_logs"
    path_logs = os.path.join(parent_dir, directory_logs)
    print(os.path.isdir(path_logs))
    if(not os.path.isdir(path_logs)):
        os.mkdir(path_logs)
    return path_plots, path_logs

def setting_logs(path_logs, num_gNBs):
    logging.basicConfig(level=logging.INFO, format='%(message)s', handlers=[logging.StreamHandler()])
    logger = logging.getLogger('Cell_loads_logger')
    log_filename = os.path.join(path_logs, "Cell_loads_log.csv")
    Cell_loads_csv_handler = logging.FileHandler(log_filename, mode='w')
    logger.addHandler(Cell_loads_csv_handler)
    header_gNB = ','.join([f'Cell_loads_{i+1}' for i in range(num_gNBs)]) + '\n'
    Cell_loads_csv_handler.stream.write(header_gNB)

    logger_stats = logging.getLogger('Cell_loads_stats_logger')
    log_filename_stats = os.path.join(path_logs, "Cell_loads_stats_log.csv")
    stats_csv_handler = logging.FileHandler(log_filename_stats, mode='w')
    logger_stats.addHandler(stats_csv_handler)
    stats_csv_handler.stream.write("Episode,Mean Cell Load 6,Std Cell Load 6\n")

    return logger, logger_stats

def log_Cell_loads(logger, Cell_loads):
    logger.info(",".join(map(str, Cell_loads)))

def log_Cell_loads_stats(logger_stats, episode, mean_load, std_load):
    logger_stats.info(f"{episode},{mean_load},{std_load}")

def plot_results(total_rewards_per_episode, path_plots, path_logs, num_gNBs):
    plt.figure(figsize=(10, 6))
    plt.plot(total_rewards_per_episode)
    plt.title("Average rewards per episode")
    plt.xlabel("Episodes")
    plt.ylabel("Average rewards")
    plot_filename = os.path.join(path_plots, "Rewards.png")
    plt.savefig(plot_filename)
    plt.show()

    log_filename = os.path.join(path_logs, "Cell_loads_log.csv")
    gNB_DF = pd.read_csv(log_filename)
    plt.figure(figsize=(10, 6))
    for i in range(num_gNBs):
        plt.plot(gNB_DF[f'Cell_loads_{i+1}'], label=f'gNB{i+1}')
    plt.title("Cell_loads vs. Time")
    plt.xlabel("Iterations")
    plt.ylabel("Cell_loads")
    plt.legend()
    plot_filename = os.path.join(path_plots, "Cell_loads.png")
    plt.savefig(plot_filename)
    plt.show()
###################################################################################################################################################

def main():

######################################################## Scenario Setting for Gym ######################################################
    port = 8999
    timestep = 0.2
    totaltime = 30

    env_id = 'NetSim-v0'
    kwargs = {
        'port': port,
        'timestep': timestep,
        'totaltime' : totaltime,
        'State_list': ["CellLoad_ex2", "EdgeUE_ex2"],
        'State_device_list': ["gNB", "gNB"],
        'State_device_range_list': [[9, 15], [9, 15]],
        'Action_list': ["HOM"],
        'Reward_list': ["Dummy"], # Actual reward is calculated based on obs
        'Reward_device_list': ["UE"],
        'Reward_device_range_list': [[16, 16]],
        'obs_space_list': [[0, 2, 7], [0, 100, 7]],
        'act_space_list': [ [0, 6, 7] ]
    }
###################################################################################################################################################

######################################################## Scenario Setting for RL ######################################################
    current_episode_num = 0 
    max_episode_num = 300
    current_time = 0

    num_gNBs = 7
    #num_UEs = 50
    serving_div = [0.9, 1.1, 1.5]
    neighbor_div = [0.5, 0.7]
    edge_div = [30, 60]
    adjustments = [-0.4, -0.2, 0, 0.2, 0.4]
    reward_div = [-0.5, -0.2, 0, 0.2, 0.5]
    num_serving_buckets = len(serving_div) + 1
    num_neighbor_buckets = len(neighbor_div) + 1
    num_edge_buckets = len(edge_div) + 1
    num_act_buckets = len(adjustments)
    num_state = num_serving_buckets * num_neighbor_buckets * num_edge_buckets
    num_actions = num_act_buckets
    q_table = np.zeros((num_state, num_actions)) 
    exploration = max_episode_num / 2
    epsilon = 1
    EPSILON_DECAY = 0.25 ** (1 / exploration)
    MIN_EPSILON = 0.25
    gamma = 0.9
    alpha = 0.3
    total_rewards_per_episode = []
    steps_per_episode = int(totaltime/timestep)
    center_cell_loads_per_episode = []
###################################################################################################################################################

    path_plots, path_logs = setting_paths()
    logger, logger_stats = setting_logs(path_logs, num_gNBs)

    env = gym.make(env_id, **kwargs)

    while current_episode_num < max_episode_num:
        total_reward = 0
        current_HOM = [3 for _ in range(num_gNBs)]
        center_cell_loads = []
        obs, info = env.reset()

        load_values = obs["CellLoad_ex2"]
        edge_values = obs["EdgeUE_ex2"]
        serving_load, neighbor_load, edge_ratio = extract_obs_data(load_values, edge_values)
        state = obs_to_state(serving_div, neighbor_div, edge_div, serving_load, neighbor_load, edge_ratio)

        current_time = timestep

        while True:
            print(f"\n{'-' * 43} {current_time:.1f} {'-' * 43}\n")   
            print("            State :")
            for key, value in obs.items():
                print(f"            - {key} : {value}")

###################################################### Action Decision (Example specific) #########################################################
            ActionValue = []
            if np.random.rand() < epsilon:
                action_index = np.random.randint(num_actions)  # Explore
            else:
                action_index = np.argmax(q_table[state])  # Exploit

            delta_h = action_to_HOM_adjustments(adjustments, action_index)
            current_HOM_center = current_HOM[6] + delta_h
            current_HOM_center = max(0, min(6, current_HOM_center))
            current_HOM_rest = 6 - current_HOM_center
            current_HOM = [current_HOM_rest] * 6 + [current_HOM_center]
            ActionValue = [current_HOM]
            print("            Action :", ActionValue )
###################################################################################################################################################

            obs, reward_list, terminated, truncated, info = env.step(ActionValue)

            if not (terminated or truncated): 
                load_values = obs["CellLoad_ex2"]
                edge_values = obs["EdgeUE_ex2"]
                next_serving_load, next_neighbor_load, next_edge_ratio = extract_obs_data(load_values, edge_values)
                next_state = obs_to_state(serving_div, neighbor_div, edge_div, next_serving_load, next_neighbor_load, next_edge_ratio)

                reward = obs_to_reward(reward_div, next_serving_load, next_neighbor_load, serving_load, neighbor_load)
                print(f"            Rewards : [{reward}]")
                print("\n")

######################################################## Q-learning ######################################################           
            q_table[state, action_index] += alpha * (reward + gamma * np.max(q_table[next_state]) - q_table[state, action_index])
            state = next_state
            serving_load = next_serving_load
            neighbor_load = next_neighbor_load
            edge_ratio = next_edge_ratio
            total_reward += reward
###################################################################################################################################################            

            center_cell_loads.append(load_values[6])
            current_time +=timestep

            if current_episode_num == max_episode_num-1:
                log_Cell_loads(logger, load_values)

            if (terminated or truncated):
                current_episode_num += 1
                
                if epsilon > MIN_EPSILON:
                    epsilon *= EPSILON_DECAY
                    epsilon = max(MIN_EPSILON, epsilon)

                total_rewards_per_episode.append(total_reward/steps_per_episode)
                mean_load_6 = np.mean(center_cell_loads)
                std_load_6 = np.std(center_cell_loads)
                center_cell_loads_per_episode.append((mean_load_6, std_load_6))
                log_Cell_loads_stats(logger_stats, current_episode_num, mean_load_6, std_load_6)

                print(f"\n  {'*' * 37} Simulation completed {'*' * 38}")
                print(f"  {'*' * 37} CurrentEpisodeNum : {current_episode_num}, Reward: {total_reward/steps_per_episode} {'*' * 37}")
                break

    env.close()
    print(f"\n{'*' * 45} Done {'*' * 46}")            
    plot_results(total_rewards_per_episode, path_plots, path_logs, num_gNBs)

if __name__ == "__main__":
    main()