#include "tetris.hpp"
#include <iostream>

static float frametime = 0;

TetrisCore::TetrisCore() : rng(rd())
{
    GenerateBag(); // Generate this bag
    GenerateBag(); // Generate next bag

    usedHold = false;
    gameOver = false;
}

void TetrisCore::HoldBlock()
{
    if (usedHold)
        return;

    if (holdBlock)
        currentBag.push_front(*holdBlock);

    holdBlock.emplace(currentBlock->GetType());
    currentBlock.reset();
    usedHold = true;
}

void TetrisCore::GenerateBag()
{
    array<Block, BAG_SIZE> newBag = {
        Block(I), Block(J), Block(L), Block(O), Block(S), Block(T), Block(Z)
    };
    shuffle(newBag.begin(), newBag.end(), rng);

    currentBag.insert(
        currentBag.end(),
        make_move_iterator(newBag.begin()),
        make_move_iterator(newBag.end())
    );
}

void TetrisCore::NextBlock()
{
    currentBlock = std::move(currentBag.front());
    currentBlock->Move(currentBlock->GetType() == O ? 4 : 3, 0);

    currentBag.pop_front();
    if (currentBag.size() == BAG_SIZE)
        GenerateBag();
}

void TetrisCore::UpdateBoard()
{
    board.LockBlock(*currentBlock);
    currentBlock.reset();
    usedHold = false;
}

bool TetrisCore::CheckValidPos(int offsetX, int offsetY)
{
    return board.CheckFit(offsetX, offsetY, *currentBlock);
}

int TetrisCore::GetHardDropPos()
{
    int i = 0;
    while (CheckValidPos(0, i++));
    return i;
}
