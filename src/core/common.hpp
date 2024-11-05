#ifndef COMMON_HPP
#define COMMON_HPP

using namespace std;
constexpr int TETROMINO_SIZE = 4;

constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 22;

constexpr int BAG_SIZE = 7;

constexpr int BLOCK_TYPES = 7;
enum BlockType {I, J, L, O, S, T, Z, EMPTY};

constexpr int ROTATION_STATES = 4;
enum RotateState {INITIAL, LEFT, DOWN, RIGHT};

struct GameStats
{
    float timeElapsed = 0;
    int droppedBlockCount = 0;
    int clearedLineCount = 0;
    int tSpinCount = 0;
    int tetrisCount = 0;
    int fullClearCount = 0;
    int keyPressed = 0;
    int comboCount = -1;
    int b2bChain = -1;
    int level = 1;
    int score = 0;
};


#endif /* COMMON_HPP */
