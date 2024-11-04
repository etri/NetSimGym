import gymnasium as gym
import NetSimGym
import numpy as np

def main():

######################################################## Scenario Setting (Example specific) ######################################################
    port = 8999
    timestep = 0.5 # Timestep must be less than half of totaltime and greater than 0.2
    totaltime = 15 # Set to match the simulation time in NetSim
    
    env_id = 'NetSim-v0'
    kwargs = {
        'port' : port,
        'timestep' : timestep, # Data exchange interval
        'totaltime' : totaltime, # Simulation time for each episode

        'State_list' : ["RandomState", "IntState"],
        'State_device_list' : ["UE", "UE"], # Device type for gathering state // Example [ "UE", "GNB" , "Node" ]
        'State_device_range_list' : [[7, 9], [7, 11]],  # Device Number Range 

        'Action_list' : ["RandomAction", "IntAction"],

        'Reward_list' : ["Dummy"],
        'Reward_device_list' : ["UE"], # Device type for gathering Reward // Example [ "UE", "GNB" , "Node" ]
        'Reward_device_range_list' : [[12, 12]],  # Device Number Range 

        'obs_space_list' : [ [0, 4, 3], [0, 4, 5]],   # [ Min , Max , DataNum ]
        'act_space_list' : [ [1, 9, 3] ] # [ Min , Max , DataNum ]
    }

    current_episode_num = 0 
    max_episode_num = 10 # Total episode number
    current_time = 0
###################################################################################################################################################

    env = gym.make(env_id, **kwargs)

    while current_episode_num < max_episode_num:
        obs, info = env.reset()
        current_time = timestep
        
        while True:
            print(f"{'-' * 45} {current_time:.1f} {'-' * 45}\n")
            print("            State :")
            for key, value in obs.items():
                print(f"            - {key} : {value}")

###################################################### Action Decision (Example specific) #########################################################
            RandomAction = np.random.randint(1, 10, size=3)
            IntAction = [1, 2]
            ActionValue = [RandomAction, IntAction]
            print("            Action :", ActionValue )
###################################################################################################################################################

            obs, reward, terminated, truncated, info = env.step(ActionValue)
            print("            Reward : ", reward)
            print("\n")

            current_time +=timestep
            
            if (terminated or truncated):
                current_episode_num += 1
                print(f"\n{'*' * 37} Simulation completed {'*' * 38}")
                print(f"{'*' * 37} CurrentEpisodeNum : {current_episode_num} {'*' * 37}")
                #time.sleep(3)
                break
    env.close()
    print(f"\n{'*' * 45} Done {'*' * 46}")

if __name__ == "__main__":
    main()