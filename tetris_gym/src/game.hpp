#ifndef GAME_HPP
#define GAME_HPP

#include <algorithm>
#include <deque>
#include <optional>
#include <random>
#include "block.hpp"
#include "board.hpp"

#define BAG_SIZE 7

enum Action {
    DO_NOTHING,
    HOLD_BLOCK,
    SOFT_DROP,
    HARD_DROP,
    MOVE_LEFT,
    MOVE_RIGHT,
    ROTATE_LEFT,
    ROTATE_RIGHT,
    ROTATE_DOWN
};

struct GameStats
{
    int droppedBlockCount = 0;
    int clearedLineCount = 0;
    int tSpinCount = 0;
    int tetrisCount = 0;
    int fullClearCount = 0;
    int comboCount = -1;
    int b2bChain = -1;
    int score = 0;

    int holes = 0;
    int highestCol = 0;
};

static GameStats initStats;

class Game
{
public:
    Game();

    int Update(Action action);
    bool IsGameOver();
    void Reset();

    Board board;
    deque<Block> currentBag;
    optional<Block> currentBlock;
    optional<Block> holdBlock;
    bool usedHold;

    GameStats stats;

private:
    //Internal
    int frameCounter;
    int lockDownMove;
    int lockDownTimer;
    bool touchedDown;
    bool tSpinDetected;
    bool normalTspin;

    random_device rd;
    mt19937 rng;

    //User input
    void Rotate(RotateState direction);
    void HardDrop();
    void SoftDrop();
    void HoldBlock();

    //Reward calculation
    int GetHoles();
    int GetHighestCol();

    //Internal
    void GenerateBag(deque<Block> &bag);
    Block NextBlock();
    void LockBlock();
    void LockDownReset();

    bool CheckValidPos(int offsetX, int offsetY);
    void ValidateTSpin();
    int GetHardDropPos();
    void MoveVertical(int lines);
    void MoveHorizontal(bool left, int col);
    bool TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset);

    template <size_t N>
    bool TryRotateOpposite(RotateState currentState, RotateState newState, 
                           const array<array<Coord, N>, 4>& srsData);
};

#endif /* GAME_HPP */
