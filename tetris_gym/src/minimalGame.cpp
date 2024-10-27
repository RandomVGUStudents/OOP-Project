#include "minimalGame.hpp"

Game::Game() : rng(rd())
{
    GenerateBag(currentBag); // Generate this bag
    GenerateBag(currentBag); // Generate next bag

    gameOver = false;

    NextBlock();
}

void Game::Reset()
{
    currentBag.clear();
    GenerateBag(currentBag); // Generate this bag
    GenerateBag(currentBag); // Generate next bag

    gameOver = false;
    currentBlock.reset();

    NextBlock();
}

int Game::GetHardDropPos()
{
    int i = 0;
    while (CheckValidPos(0, i++));
    return i;
}

int Game::PlaceBlock(int col, RotateState rotateState)
{
    int reward = stats.score;

    currentBlock->Rotate(rotateState);

    // Check valid move
    if (!CheckValidPos(col, 0))
    {
        reward = -2000;
    }
    else
    {
        currentBlock->Move(col, 0);
        int droppedLine = GetHardDropPos() - BOARD_HEIGHT + 20;

        stats.score += 2 * droppedLine;
        currentBlock->Move(0, droppedLine);

        LockBlock();
        NextBlock();

        reward = stats.score - reward;
    }

    return reward;
}

void Game::GenerateBag(deque<Block> &bag)
{
    array<Block, BAG_SIZE> newBag = {
        Block(I), Block(J), Block(L), Block(O), Block(S), Block(T), Block(Z)
    };

    shuffle(newBag.begin(), newBag.end(), rng);
    bag.insert(bag.end(), make_move_iterator(newBag.begin()), make_move_iterator(newBag.end()));
}

void Game::NextBlock()
{
    currentBlock = std::move(currentBag.front());
    currentBag.pop_front();

    if (currentBag.size() == BAG_SIZE)
        GenerateBag(currentBag);
}

void Game::LockBlock()
{
    board.LockBlock(*currentBlock);
    currentBlock.reset();
    NextBlock();

    stats.droppedBlockCount++;

    int clearedLine = board.CheckFullRow();
    if (clearedLine == 0)
    {
        stats.comboCount = -1;
        return;
    }

    stats.comboCount++;
    stats.clearedLineCount += clearedLine;
    stats.score += stats.comboCount * 50;

    int baseScore = 0;

    switch (clearedLine)
    {
        case 1: baseScore = 100; stats.b2bChain = -1; break;
        case 2: baseScore = 300; stats.b2bChain = -1; break;
        case 3: baseScore = 500; stats.b2bChain = -1; break;
        case 4: baseScore = 800; stats.tetrisCount++; stats.b2bChain++; break;
    }

    
    if (stats.b2bChain > 0)
    {
        baseScore *= 1.5;
    }

    if (board.CheckFullClear())
    {
        stats.fullClearCount++;
        switch (clearedLine) {
            case 1: baseScore += 800; break;
            case 2: baseScore += 1200; break;
            case 3: baseScore += 1800; break;
            case 4: baseScore += (stats.b2bChain > 0) ? 3200 : 2000; break;
        }
    }

    stats.score += baseScore;
}

bool Game::CheckValidPos(int offsetX, int offsetY)
{
    return board.CheckFit(offsetX, offsetY, *currentBlock);
}
