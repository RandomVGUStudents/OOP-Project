#ifndef GAME_HPP
#define GAME_HPP

#include <algorithm>
#include <deque>
#include <optional>
#include <random>
#include "board.hpp"
#include "block.hpp"

#define BAG_SIZE 7

struct GameStats
{
    int droppedBlockCount = 0;
    int clearedLineCount = 0;
    int tetrisCount = 0;
    int fullClearCount = 0;
    int comboCount = -1;
    int b2bChain = -1;
    int score = 0;
};

class Game
{
public:
    Game();

    void Reset();

    void HoldBlock();
    int PlaceBlock(int col, RotateState rotateState);

    Board board;
    deque<Block> currentBag;
    optional<Block> currentBlock;
    GameStats stats;
    bool gameOver;
private:

    random_device rd;
    mt19937 rng;

    //Internal
    void GenerateBag(deque<Block> &bag);
    void NextBlock();
    void LockBlock();

    int GetHardDropPos();
    bool CheckValidPos(int offsetX, int offsetY);
};

#endif /* GAME_HPP */
