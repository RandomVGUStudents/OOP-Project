#ifndef GAME_HPP
#define GAME_HPP

#include <algorithm>
#include <deque>
#include <random>
#include "board.hpp"


class TetrisCore
{
public:
    TetrisCore();

    GameStats stats;

    virtual void NewGame();
    const bool IsOver();

protected:
    Board board;
    deque<BlockType> currentBag;
    Block currentBlock;
    Block holdBlock;
    bool usedHold;
    bool gameOver;

    random_device rd;
    mt19937 rng;

    void GenerateBag();
    void NextBlock();
    virtual void HoldBlock();

    void UpdateBoard();

    bool CheckValidPos(int offsetX, int offsetY);
    int GetHardDropPos();
};

#endif /* GAME_HPP */
