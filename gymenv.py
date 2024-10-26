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
    def __init__(self, render_mode: Optional[str] = None):
        super().__init__()
        self.env = Tetris()
        
        self.observation_space = spaces.Dict({
            "board": spaces.Box(0, 1, (10, 20), np.uint8),
            "queue": spaces.Box(0, 6, (6,), np.uint8),
            "hold": spaces.Box(0, 7, (1,), np.uint8),
            #spaces.Discrete(7),
            #spaces.Discrete(8)
        })
        
        self.action_space = spaces.Discrete(41)
        self.render_mode = render_mode


    def _get_obs(self):
        state = self.env.getState()
        obs = {
            "board": state[0],
            "queue": np.insert(state[1], 0, state[2]),
            "hold": np.array([state[3]], dtype=np.uint8)
        }

        return obs
    
    def step(self, action):
        if action == 40:
            action_dict = {
                'use_hold': True,
                'col': None,
                'rotation': None
            }
        else:
            rotation = action // 10
            col = action % 10 - 1

            action_dict = {
                'use_hold': False,
                'col': col,
                'rotation': rotation
            }
        
        if self.render_mode == "human":
            self.render(action_dict)
        reward, done, info = self.env.step(action_dict)
        if self.render_mode == "human":
            self.render()
        return self._get_obs(), reward, done, False, info

    def render(self, action_dict: Optional[dict] = None):
        obs = self._get_obs()

        # Mapping for Tetris blocks and empty cell
        block_map = {
            0: "I",
            1: "J",
            2: "L",
            3: "O",
            4: "S",
            5: "T",
            6: "Z",
            7: "None",  # Blank
        }

        # Render the board
        board = obs['board'].T
        print("Tetris Board:")
        for row in board:
            print("|" + "".join("  " if cell == 0 else "██" for cell in row) + "|")

        # Render hold and queue
        hold = obs['hold'][0]
        queue = obs['queue']
    
        print("\nHold: " + block_map[hold] if hold in block_map else " ")
        print("Queue: " + " ".join(block_map[block] for block in queue if block in block_map))
        if action_dict is not None:
            if action_dict['use_hold']:
                print("Action: Hold piece")
            else:
                print(
                    "Action: Placing piece "
                        + block_map[queue[0]]
                        + " at col = "
                        + str(action_dict['col'])
                        + " and rotation "
                        + str(action_dict['rotation']))
    
    def reset(self, seed: Optional[int] = None, options: Optional[dict] = None):
        super().reset(seed=seed)
        return self._get_obs(), {}

gym.register(
    id="w/TetrisEnv",
    entry_point=ModernTetrisGymEnv,
)
