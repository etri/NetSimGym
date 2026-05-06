import ray
from ray.rllib.algorithms.ppo import PPOConfig
from ray import tune
from ray.tune.registry import register_env

from PortManager import PortManagerActor
from NetSimGym.netsimenv2_discrete import NetSimEnv2Discrete
import os
import numpy as np

from Auto_NetSim_RL_portnum_iternum import AutoNetSim
import multiprocessing

# setting
os.environ["RAY_DEDUP_LOGS"] = "0"
os.environ["USE_LIBUV"] = "0"

num_env_runners_run = 4
num_learners_run = 1
training_iteration = 80

timestep = 0.2
totaltime = 30
State_list = ["CellLoad_ex2" , "EdgeUE_ex2"]
State_device_list = ["gNB" , "gNB"]
State_device_range_list = [[9, 15], [9, 15]]
Action_list = ["HOM"]
Reward_list = ["Dummy"]
Reward_device_list = ["UE"]
Reward_device_range_list = [[16, 16]]

# Discrete state dividers (from example2)
serving_div = [0.9, 1.1, 1.5]
neighbor_div = [0.5, 0.7]
edge_div = [30, 60]

# Discrete action adjustments (from example2)
adjustments = [-0.2, -0.1, 0, 0.1, 0.2]

totalstep_ratio = totaltime/timestep
if np.isclose(np.mod(totaltime, timestep), 0) or np.isclose(np.mod(totaltime, timestep), timestep):
    totalstep = int(totalstep_ratio) -1
else: 
    totalstep = int(np.floor(totalstep_ratio))
totalstep_for_batch = totalstep-1
print ("step for train batch size is total step minus one:",totalstep_for_batch)

def env_creator(env_config):
    port = ray.get(port_manager_actor.get_next_port.remote())
    env_config['port'] = port
    env_config['timestep'] = timestep
    env_config['totaltime'] = totaltime
    env_config['State_list'] = State_list
    env_config['State_device_list'] = State_device_list
    env_config['State_device_range_list'] = State_device_range_list
    env_config['Action_list'] = Action_list
    env_config['Reward_list'] = Reward_list
    env_config['Reward_device_list'] = Reward_device_list
    env_config['Reward_device_range_list'] = Reward_device_range_list
    # Discrete spaces
    env_config['serving_div'] = serving_div
    env_config['neighbor_div'] = neighbor_div
    env_config['edge_div'] = edge_div
    env_config['adjustments'] = adjustments
    env = NetSimEnv2Discrete(env_config=env_config)
    return env

total_cpu = multiprocessing.cpu_count()
half_cpu_int = total_cpu // 2
max_exponent = np.floor(np.log2(half_cpu_int))
max_env_runner = 2 ** int(max_exponent)
max_learner = max_env_runner

auto_netsim = AutoNetSim()

train_batch_size_run = totalstep_for_batch*max_env_runner*2
if num_learners_run == 0:
    train_batch_size_per_learner_run = train_batch_size_run
else:
    train_batch_size_per_learner_run = train_batch_size_run/num_learners_run
train_round_per_runner_run = max_env_runner*2/num_env_runners_run*training_iteration

# add autorun iteration for stability
for j in range(num_env_runners_run):
    port_netsim = 9000 + j
    print (port_netsim, train_round_per_runner_run)
    auto_netsim.NetSimAutoRun(port_netsim, int(train_round_per_runner_run+10))
port_netsim_eval = port_netsim + 1
auto_netsim.NetSimAutoRun(port_netsim_eval, int(training_iteration+10))

port_manager_actor = PortManagerActor.remote(9000, 9100)
register_env("my_custom_NetSimEnv2", env_creator)

config=(
    PPOConfig()
    .python_environment()
    .resources()
    .framework()
    .environment(env="my_custom_NetSimEnv2", env_config = {})
    .env_runners(
        num_env_runners = num_env_runners_run,
        rollout_fragment_length=totalstep_for_batch,
        remote_worker_envs=False,
        sample_timeout_s = 1500
        #num_cpus_per_env_runner=0,
        #num_gpus_per_env_runner=0
        #explore=false for evaluation
    )
    .learners(num_learners=num_learners_run)
    #learners(num_learners=2) #libuv error
    .training(        
        #lr=[(0, 3e-4), (1e4, 1e-4)],
        lr=3e-4,
        train_batch_size_per_learner=train_batch_size_per_learner_run,
        minibatch_size=totalstep_for_batch*2,
        num_epochs = 30 #for longer learning time
    ) #ppo setting is here
    .callbacks()
    # explore() #not compatible with RLModule
    .api_stack(
        enable_rl_module_and_learner=False,
        enable_env_runner_and_connector_v2=False,
    )
    # multi_agent() #not using for my work
    # offline_data() #not using for my work
    .evaluation(
        evaluation_interval=1,
        evaluation_duration=1,
        evaluation_duration_unit="episodes" #timestep
        #evaluation_parallel_to_training
    )
    .reporting()
    .checkpointing(
        export_native_model_files=True
    )
    .debugging()
    .fault_tolerance()
    .experimental()
)

CHECKPOINT_PATH = "C:\\Users\\leegh\\Desktop\\runconfig_load\\r4l1\\PPO_2026-04-11_17-27-40\\PPO_my_custom_NetSimEnv2_4dfb4_00000_0_2026-04-11_17-27-40\\checkpoint_000023"
RESTORE_FROM_CHECKPOINT = True  # 체크포인트에서 이어서 학습할지 여부

if RESTORE_FROM_CHECKPOINT:
    algo = config.build()
    algo.restore(CHECKPOINT_PATH)
    print(f"Restored from checkpoint: {CHECKPOINT_PATH}")

    storage_path = f"C:\\Users\\leegh\\Desktop\\runconfig_load\\r{num_env_runners_run}l{num_learners_run}_continued"
    os.makedirs(storage_path, exist_ok=True)

    for i in range(training_iteration):
        result = algo.train()
        print(f"[Iter {i+1}/{training_iteration}] reward_mean={result.get('env_runners', {}).get('episode_reward_mean', 'N/A')}")
        if (i + 1) % 5 == 0:
            checkpoint = algo.save(storage_path)
            print(f"Checkpoint saved: {checkpoint}")

    algo.stop()
else:
    config_dict = config.to_dict()
    tuner = tune.Tuner(
        "PPO",
        param_space=config_dict,
        run_config=tune.RunConfig(
            storage_path=f"C:\\Users\\leegh\\Desktop\\runconfig_load\\r{num_env_runners_run}l{num_learners_run}",
            stop={"training_iteration": training_iteration},
            verbose=1,
            checkpoint_config=tune.CheckpointConfig(checkpoint_frequency=5)
        )
    )
    result = tuner.fit()
