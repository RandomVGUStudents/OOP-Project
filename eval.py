import sys
import gymenv
from rl_zoo3.enjoy import enjoy

def main():
    sys.argv = [
        "python",
        "-f", "./logs/",
        "--algo", "ppo",
        "--env", "w/TetrisEnv",
        #"--optimize",
        #"--n-jobs", "2",
        #"--study-name", "tetris-agent-tuning",
        #"--optimization-log-path", "./tetris_agent_tuning_log/",
        "--progress"
    ]
    enjoy()

if __name__ == "__main__":
    main()
