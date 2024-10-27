import sys
import gymenv
from rl_zoo3.train import train

def main():
    sys.argv = [
        "python",
        "--algo", "ppo",
        "--env", "w/TetrisEnv",
        "-c", "hyperparams.yml",
        "-tb", "./tetris_tensorboard/",
        #"--optimize",
        #"--n-jobs", "2",
        #"--study-name", "tetris-agent-tuning",
        #"--optimization-log-path", "./tetris_agent_tuning_log/",
        "--progress"
    ]
    train()

if __name__ == "__main__":
    main()
