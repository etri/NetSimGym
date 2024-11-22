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
- Gymnasium 14.1.15
- Protobuf 5.27.2
- numpy 1.26.4
- pandas 2.2.2
- PyGetWindow 0.0.9
- PyAutoGUI 0.9.54
- matplotlib 3.8.3

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

- Running NetSim simulation:
  - In "Your Work," select either "NetSimGym_ex1_3x6" or "NetSimGym_ex2-2_ran3_30s"
  - Click "Run Simulation" to start
- You need to click "Run Simulation" for each episode.
  - Open CMD and navigate to the directory containing the agent files
  
    $ cd C:\Users\ETRI\Documents\NetSim_Work_2024\python
  
  - Run the agent file with the same number as the currently running NetSim simulation example
  
    $ python agent_ex1.py or python agent_ex2.py

### Running simulation via CMD with Python files

- Running NetSim simulation:
  - Edit "NetSim_Scenario_ex1.py" and 'NetSim_Scenario_ex2,py" file (set netsim path to the "bin_x64" directory of your workspace, IOPath to the experiment folder location, and license path to the license file name)
  - ex)
    - NETSIM_PATH = "C:\\Users\\ETRI\\Documents\\NetSim\\Workspaces\\NetSimGym_v1-0\\bin_x64"
    - IO_PATH = "C:\\Users\\ETRI\\Documents\\NetSim\\Workspaces\\NetSimGym_v1-0"
    - LICENSE_PATH = os.path.join(NETSIM_PATH, "netsim-cloud-license-etri-korea.lic")
  - Navigate to the directory containing the NetSim_Scenario file
    $ cd C:\Users\ETRI\Documents\NetSim_Work_2024\python
  - Run the NetSim Scenario file
    $ python NetSim_Scenario_ex1.py or python NetSim_Scenario_ex2.py

## Acknowledgement

This work was supported by Institute of Information & Communications Technology Planning & Evaluation (IITP) grant funded by the Korea government (MSIT) (RS-2024-00392332, Development of 6G Network Integrated Intelligence Plane Technologies, 50% & 2022-0-00862, Development of Intelligent 6G Mobile Core Network Technologies, 50%). 
