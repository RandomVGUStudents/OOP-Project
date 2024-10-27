from typing import Optional
import gymnasium as gym
import numpy as np
from gymnasium import spaces
from tetris_gym import Tetris

class ModernTetrisGymEnv(gym.Env):
    metadata = {
        "render_modes": ["human"],
        "render_fps": 60
    }
    
    def __init__(self, render_mode: Optional[str] = None, max_steps: int = 2*1e3):
        super().__init__()
        self.env = Tetris()
        self.observation_space = spaces.Dict({
            "board": spaces.Box(0, 1, (10, 22), np.uint8),
            "queue": spaces.Box(0, 6, (6,), np.uint8),
            "hold": spaces.Box(0, 7, (1,), np.uint8),
        })
        self.action_space = spaces.Discrete(9)
        self.render_mode = render_mode
        self.episode_length = 0
        self.max_steps = max_steps

    def _get_obs(self):
        state = self.env.getState()
        obs = {
            "board": state[0],
            "queue": state[1],
            "hold": np.array([state[2]], dtype=np.uint8)
        }
        return obs

    def step(self, action):
        self.episode_length += 1

        reward, done, info = self.env.step(action)

        info['episode_length'] = self.episode_length

        truncated = False
        if self.episode_length >= self.max_steps:
            truncated = True

        if done or truncated:
            info["final_episode_length"] = self.episode_length

        if self.render_mode == "human":
            self.render()

        return self._get_obs(), reward, done, truncated, info

    def render(self):
        obs = self._get_obs()

        block_map = {0: "I", 1: "J", 2: "L", 3: "O", 4: "S", 5: "T", 6: "Z", 7: "None"}
        board = obs['board'].T

        print("Tetris Board:")
        for row in board:
            print("|" + "".join("  " if cell == 0 else "██" for cell in row) + "|")

        hold = obs['hold'][0]
        queue = obs['queue']
        print("\nHold: " + block_map[hold] if hold in block_map else " ")
        print("Queue: " + " ".join(block_map[block] for block in queue if block in block_map))

    def reset(self, seed: Optional[int] = None, options: Optional[dict] = None):
        super().reset(seed=seed)
        self.episode_length = 0
        self.env.reset()

        if self.render_mode == "human":
            self.render()

        return self._get_obs(), {}

gym.register(
    id="w/TetrisEnv",
    entry_point=ModernTetrisGymEnv,
)
