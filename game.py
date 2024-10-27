import gymnasium as gym
import gymenv
import numpy as np
import sys
import termios
import tty
import select
import time
from enum import IntEnum

class Action(IntEnum):
    DO_NOTHING = 0
    HOLD_BLOCK = 1
    SOFT_DROP = 2
    HARD_DROP = 3
    MOVE_LEFT = 4
    MOVE_RIGHT = 5
    ROTATE_LEFT = 6
    ROTATE_RIGHT = 7
    ROTATE_DOWN = 8

class NonBlockingInput:
    def __init__(self):
        self.old_settings = termios.tcgetattr(sys.stdin)
        
    def __enter__(self):
        tty.setcbreak(sys.stdin.fileno())
        return self
        
    def __exit__(self, type, value, traceback):
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, self.old_settings)
        
    def get_char_if_available(self):
        if select.select([sys.stdin], [], [], 0)[0]:
            return sys.stdin.read(1)
        return None

class TetrisGame:
    def __init__(self):
        self.env = gym.make("w/TetrisEnv", render_mode="human")
        self.env.reset()
        self.last_action_time = time.time()
        self.action_delay = 1.0  # 1 second delay for DO_NOTHING
        
        # Key mappings
        self.key_actions = {
            'h': Action.MOVE_LEFT,    # vim-style left
            'l': Action.MOVE_RIGHT,   # vim-style right
            'j': Action.SOFT_DROP,    # vim-style down
            'k': Action.ROTATE_RIGHT, # vim-style up
            ' ': Action.HARD_DROP,    # space for hard drop
            'z': Action.ROTATE_LEFT,  # z for rotate left
            'a': Action.ROTATE_DOWN,  # a for rotate down
            'c': Action.HOLD_BLOCK,   # c for hold block
        }

    def print_controls(self):
        print("\nControls:")
        print("h/l - Move Left/Right")
        print("j   - Soft Drop")
        print("k   - Rotate Right")
        print("z   - Rotate Left")
        print("a   - Rotate Down")
        print("c   - Hold Block")
        print("space - Hard Drop")
        print("q   - Quit")
        print("\nStarting game...\n")

    def get_action_from_keyboard(self, input_handler):
        char = input_handler.get_char_if_available()
        
        if char is None:
            current_time = time.time()
            if current_time - self.last_action_time >= self.action_delay:
                self.last_action_time = current_time
                return Action.DO_NOTHING
            return None
            
        self.last_action_time = time.time()
        
        if char == 'q':  # Quit game
            return 'QUIT'
            
        return self.key_actions.get(char, None)

    def run(self):
        self.print_controls()
        
        with NonBlockingInput() as input_handler:
            running = True
            while running:
                action = self.get_action_from_keyboard(input_handler)
                
                if action == 'QUIT':
                    running = False
                elif action is not None:
                    observation, reward, terminated, truncated, info = self.env.step(action)
                    
                    if terminated or truncated:
                        self.env.reset()
                
                # Small sleep to prevent CPU hogging
                time.sleep(0.01)

    def close(self):
        self.env.close()

def main():
    game = TetrisGame()
    try:
        game.run()
    except KeyboardInterrupt:
        print("\nExiting game...")
    finally:
        game.close()

if __name__ == "__main__":
    main()
