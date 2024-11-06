#include "tetris.hpp"
#include "core/common.hpp"

TetrisCore::TetrisCore() : rng(rd())
{
    NewGame();
}

const bool TetrisCore::IsOver()
{
    return gameOver;
}

void TetrisCore::HoldBlock()
{
    if (usedHold)
        return;

    if (holdBlock)
        currentBag.push_front(holdBlock.GetType());

    currentBlock.ResetPosition();
    holdBlock = std::move(currentBlock);
    NextBlock();

    usedHold = true;
}

void TetrisCore::GenerateBag()
{
    std::array<int, BAG_SIZE> vec = {0, 1, 2, 3, 4, 5, 6};
    std::shuffle(vec.begin(), vec.end(), rng);

    for (int num : vec)
        currentBag.push_back(BlockType(num));
}

void TetrisCore::NextBlock()
{
    auto& type = currentBag.front();
    currentBlock = std::move(Block(type));
    currentBlock.Move(type == O ? 4 : 3, 0);

    currentBag.pop_front();
    if (currentBag.size() == BAG_SIZE)
        GenerateBag();
}

void TetrisCore::UpdateBoard()
{
    board.LockBlock(currentBlock);
    NextBlock();

    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        if (board.GetBoard()[i][2] != EMPTY)
        {
            gameOver = true;
            break;
        }

    usedHold = false;
}

void TetrisCore::NewGame()
{
    currentBag.clear();
    holdBlock = Block();
    board.Init();

    GenerateBag();
    GenerateBag();

    usedHold = false;
    gameOver = false;
    stats = GameStats();

    NextBlock();
}

bool TetrisCore::CheckValidPos(int offsetX, int offsetY)
{
    return board.CheckFit(offsetX, offsetY, currentBlock);
}

int TetrisCore::GetHardDropPos()
{
    int i = 0;
    while (CheckValidPos(0, i++));
    return i - BOARD_HEIGHT + 20;
}
