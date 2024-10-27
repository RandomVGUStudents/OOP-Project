import gymnasium as gym
import numpy as np
import gymenv

env = gym.make("w/TetrisEnv", render_mode="human")

#print(env.observation_space.sample())

#print(env.reset())
env.reset()
#print(env.step(env.action_space.sample()))
for i in range(10):
    env.step(env.action_space.sample())

#env.step([0, 0, 0, 0])
