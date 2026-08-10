# NetSimGym

NetSimGym is a wrapper library to interwork [Gymnasium](https://gymnasium.farama.org/) (Gym) and [NetSim](https://tetcos.com/). 

It is designed for users who want to apply reinforcement learning (RL) in NetSim simulation. 

NetSimGym exploits **[Protocol Buffers](https://protobuf.dev/)** to interconnect Gym and NetSim to each other. 

We recommend you to understand Gym and NetSim before using NetSimGym. 

## Installation

As pre-requisite, you should have Python and NetSim: 
- Python 3.12+
- NetSim standard v14.1.15

And, you have following Python packages:
- Gymnasium 1.1.1
- Protobuf 5.27.2
- numpy 1.26.4
- pandas 2.2.2
- PyGetWindow 0.0.9
- PyAutoGUI 0.9.54
- matplotlib 3.8.3
- rllib (ray) 2.42.1

After installing required packages, clone this repository on your machine and import experiment set-up file for NetSimGym:
- Open NetSim and select "Import" from "Your Work"
- In the "Import" field, choose "Experiments/Workspace" file
- In the "Source" field, select "NetSimGym_v1-0.netsimexp" file
- In the "Destination" field, select "Create new Workspace and import experiments into the new Workspace"
- Then, set the Workspace name and location (ex: "NetSimGym_v-0 for workspace name and "C:\Users\ETRI\Documents\NetSim\Workspaces"
- Locate agent files and NetSimGym directory in the same parent directory (ex: Place "agent_ex1.py," "agent_ex2.py", and "NetSimGym" directory under C:\Users\ETRI\Documents\NetSim_Work_2024\python )

## Running Examples

There are two methods for running NetSim simulation: 

### Running simumation via NetSim GUI

Running NetSim simulation:
- In "Your Work," select either "NetSimGym_ex1_3x6" or "NetSimGym_ex2-2_ran3_30s"
- Click "Run Simulation" to start

You need to click "Run Simulation" for each episode.
Open CMD and navigate to the directory containing the agent files
  
    $ cd C:\Users\ETRI\Documents\NetSim_Work_2024\python

Run the agent file with the same number as the currently running NetSim simulation example
  
    $ python agent_ex1.py or python agent_ex2.py

### Running simulation via CMD with Python files

Running NetSim simulation:
- Edit "NetSim_Scenario_ex1.py" and 'NetSim_Scenario_ex2,py" file (set netsim path to the "bin_x64" directory of your workspace, IOPath to the experiment folder location, and license path to the license file name)
  - ex)
  - NETSIM_PATH = "C:\\Users\\ETRI\\Documents\\NetSim\\Workspaces\\NetSimGym_v1-0\\bin_x64"
  - IO_PATH = "C:\\Users\\ETRI\\Documents\\NetSim\\Workspaces\\NetSimGym_v1-0"
  - LICENSE_PATH = os.path.join(NETSIM_PATH, "netsim-cloud-license-etri-korea.lic")

Navigate to the directory containing the NetSim_Scenario file

    $ cd C:\Users\ETRI\Documents\NetSim_Work_2024\python
    
Run the NetSim Scenario file

    $ python NetSim_Scenario_ex1.py or python NetSim_Scenario_ex2.py

Run the agent file with the same number as the currently running NetSim simulation example
  
    $ python agent_ex1.py or python agent_ex2.py

## Running NetsimGym with RLlib 

Import experiment set up file using rllib.netsimexp in RLlib directory:
- Follow import experiment set-up part (line 27~33) using rllib.netsimexp

- Edit NetSim_Scenario_RL_portnum file:
    - Edit PATH using directory path and name set in netsim folder
    - ex)

    - NETSIM_PATH = "C:\\Users\\Yerin\\Documents\\NetSim\\Workspaces\\${Netsim directory name}\\bin_x64"
    - IO_PATH = "C:\\Users\\Yerin\\Documents\\NetSim\\Workspaces\\${Netsim directory name}"
    - LICENSE_PATH = os.path.join(NETSIM_PATH, "netsim-cloud-license-etri-korea.lic")


- Edit which file to run using CONFIG_FOLDER variable
- ex)
    - CONFIG_FOLDER = os.path.join(IO_PATH, "NetSimGym_ex2-2_ran3_30s")

    - Running DownLink Power Control example, use RL_based_power_control 3x6
    - Running Load balancing example, use NetsimGym_ex2-2_ran3_30s
    

Run example in rllib directory

```
$ python PPO_ex1.py or python PPO_ex2_discrete.py
 ```

## RLlib scenario modification
- For state & action & reward modification, change "env_config" variable in "PPO_ex1.py" or "PPO_ex2_discrete.py" 
- For modifying number of runners & learners, change "num_env_runners_run" and "num_learners_run" variable in "PPO_ex1.py" or "PPO_ex2_discrete.py" 
- For modifying number of iterations, change "training_iteration" variable in "PPO_ex1.py" or "PPO_ex2_discrete.py" 


## Performance metrics

<img width="700" height="400" alt="image" src="https://github.com/user-attachments/assets/23918f8c-b9f9-4a35-97b4-847cb517442d" />

### Sampling throughput & Execution time

- Sampling throughput represents the average number of environment steps collected per second. Increasing the number of runners from one to two and four increases the throughput from 3.91 steps/s to 7.87 and 14.72 steps/s, respectively. Relative to the single-runner configuration, four runners therefore achieve a 3.76 times increase in sampling throughput. The corresponding total run time decreases from 25.68 min to 13.25 and 7.68 min, resulting in 1.94 times and 3.34 times speedups. The derived parallel efficiencies are 96.9% and 83.6% for two and four runners, respectively. These results show that trajectory collection scales close to proportionally within the evaluated range, whereas end-to-end execution exhibits modest diminishing returns because not all execution stages benefit equally from additional runners.

<img width="700" height="600" alt="image" src="https://github.com/user-attachments/assets/a99a79d3-a126-4782-b4b9-0e56e166cd82" />

### System resource utilization

- System resource utilization. CPU and memory utilization were monitored at the whole-system level throughout each profiling run. CPU utilization generally increases with the number of runners, from approximately 10-12% with one runner to 16-18% with four runners. Memory utilization shows a more moderate increase, reaching approximately 66-67% with four runners. The results demonstrate that higher sampling throughput incurs additional host-resource usage, while the evaluated configurations remain within the available CPU and memory capacity of the test platform. These measurements represent system-level utilization and should not be interpreted as the resource consumption of NetSimGym alone.

### Convergence and stability

<img width="2000" height="1040" alt="image" src="https://github.com/user-attachments/assets/67813a2c-a305-43b9-b5b6-31bde8121ae0" />

- The figure compares the learning performance obtained using one and four environment runners. The L1 norm is defined as the sum of the absolute differences between each base station’s load and the average load; therefore, a lower L1 norm indicates a more balanced load distribution across the base stations. When a single runner is used, the L1 norm decreases only gradually and exhibits substantial fluctuations throughout training, reaching approximately 6.33 after 35 hours. In contrast, the four-runner configuration shows a much faster and more consistent reduction in the L1 norm, converging to approximately 4.44 within 30 hours. Its moving-average curve also becomes smoother and more stable toward the end of training. These results demonstrate that using four runners accelerates convergence, improves training stability than using a single runner.
    
## Acknowledgement

This work was supported by Institute of Information & Communications Technology Planning & Evaluation (IITP) grant funded by the Korea government (MSIT) (RS-2024-00392332, Development of 6G Network Integrated Intelligence Plane Technologies, 50% & 2022-0-00862, Development of Intelligent 6G Mobile Core Network Technologies, 50%). 
