#include "heuristics.hpp"
#include <iostream>

TetrisHeurAI::TetrisHeurAI()
    : renderer(stats, {{ SCORE, TIME, LINESPEED, BLOCKCOUNT, CUSTOM }})
{};

void TetrisHeurAI::Update()
{
    bool useHold = false;
    int move = -1;

    GameStats origStats = stats;

    currentBlock.ResetPosition();
    FindBestMove(useHold, move);

    stats = origStats;

    if (useHold)
        HoldBlock();

    RotateState rotation = INITIAL;
    int posX = 0;

    ParseMove(currentBlock.GetType(), move, rotation, posX);

    currentBlock.ResetPosition();
    MakeMove(rotation, posX);
    stats.droppedBlockCount++;

    auto now = chrono::steady_clock::now();
    stats.timeElapsed = now - stats.startTime;
}

void TetrisHeurAI::Draw(const string& customTitle, const string& customData, const string& customSubData)
{
    renderer.UpdateScreenSize();
    renderer.DrawHoldBox(holdBlock);
    renderer.DrawQueueColumn(currentBag);
    renderer.DrawBoard(currentBlock, GetHardDropPos(), board);
    renderer.DrawStats();
    renderer.DrawMessages();
    if (customTitle != "")
        renderer.DrawCustomStats(4, customTitle, customData, customSubData);
}

void TetrisHeurAI::UpdateHeuristics(HeuristicsWeights newWeights)
{
    weights = newWeights;
}

void TetrisHeurAI::FindBestMove(bool& useHold, int& move)
{
    int bestRewardNoHold = -INFINITY;
    int bestRewardHold = -INFINITY;
    int bestMoveNoHold = -1;
    int bestMoveHold = -1;

    // Case 1: No held piece.
    // -> Current piece + next piece < next piece + 2nd next piece ? hold : normal
    //
    // Case 2: Held piece exists.
    // -> The held piece + next piece > current piece + next piece ? hold : normal

    Block currentBlock = this->currentBlock;
    Block nextBlock(currentBag[0]);
    Block secondNextBlock(currentBag[1]);
    Block holdBlock = this->holdBlock;
    
    if (!holdBlock)
    {
        TryMoves(currentBlock, nextBlock, bestRewardNoHold, bestMoveNoHold);
        TryMoves(nextBlock, secondNextBlock, bestRewardHold, bestMoveHold);
    }
    else
    {
        TryMoves(currentBlock, nextBlock, bestRewardNoHold, bestMoveNoHold);
        TryMoves(holdBlock, nextBlock, bestRewardHold, bestMoveHold);
    }

    if (bestRewardNoHold > bestRewardHold)
    {
        useHold = false;
        move = bestMoveNoHold;
    }
    else
    {
        useHold = true;
        move = bestMoveHold;
    }
}

int TetrisHeurAI::SimulateMove(Block& block, RotateState s, int posX)
{
    block.Rotate(s);
    block.Move(posX, 0);

    int hardDrop = 0;
    while (board.CheckFit(0, hardDrop++, block));

    if (hardDrop <= 1)
        return -1e5;

    block.Move(0, hardDrop - 2);
    board.LockBlock(block);

    return CalcReward();
}

void TetrisHeurAI::TryMoves(Block& firstBlock, Block& secondBlock, int& bestReward, int& bestMove)
{
    BlockType firstType = firstBlock.GetType();
    BlockType secondType = secondBlock.GetType();

    RotateState tryRotation = INITIAL;
    int tryPosX = 0;

    int reward = -INFINITY;
    int move = -1;

    for (size_t i = 0; i < placePositions[firstType]; ++i)
    {
        Board orig1 = board;
        ParseMove(firstType, i, tryRotation, tryPosX);
        int firstReward = SimulateMove(firstBlock, tryRotation, tryPosX);

        for (size_t j = 0; j < placePositions[secondType]; ++j)
        {
            Board orig2 = board;
            ParseMove(secondType, j, tryRotation, tryPosX);
            int secondReward = SimulateMove(secondBlock, tryRotation, tryPosX);

            if (firstReward + secondReward > reward)
            {
                reward = firstReward + secondReward;
                move = i;
            }

            board = orig2;
            secondBlock.ResetPosition();
        }

        board = orig1;
        firstBlock.ResetPosition();
    }

    bestReward = reward;
    bestMove = move;
}
