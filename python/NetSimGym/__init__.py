import gymnasium as gym
from gymnasium.envs.registration import register

register(
    id='NetSim-v0',
    entry_point='NetSimGym.netsimenv:NetSimEnv',
)