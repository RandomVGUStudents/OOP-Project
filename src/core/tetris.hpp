#ifndef GAME_HPP
#define GAME_HPP

#include <algorithm>
#include <deque>
#include <optional>
#include <random>
#include "board.hpp"

static const int BAG_SIZE = 7;

class TetrisCore
{
public:
    TetrisCore();

protected:
    Board board;
    deque<Block> currentBag;
    optional<Block> currentBlock;
    optional<Block> holdBlock;
    bool usedHold;
    bool gameOver;

    random_device rd;
    mt19937 rng;

    void GenerateBag();
    void NextBlock();
    void HoldBlock();

    void UpdateBoard();

    bool CheckValidPos(int offsetX, int offsetY);
    int GetHardDropPos();
};

#endif /* GAME_HPP */
