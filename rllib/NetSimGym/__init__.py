import gymnasium as gym
from gymnasium.envs.registration import register

register(
    id='NetSim-v1',
    entry_point='NetSimGym.netsimenv2:NetSimEnv2',
)