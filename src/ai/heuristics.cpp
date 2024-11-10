#include "heuristics.hpp"

TetrisHeurAI::TetrisHeurAI()
    : renderer(stats, {{ SCORE, TIME, LINESPEED, BLOCKCOUNT, CUSTOM }})
    , pps(0.0)
    , timer(0)
    , cacheExists(false)
    , cachedReward(0.0)
    , cachedMove(0)
    , cachedRotation(INITIAL)
{};

void TetrisHeurAI::Update()
{
    auto now = chrono::steady_clock::now();
    stats.timeElapsed = now - stats.startTime;

    if (pps != 0)
    {
        int millisec = static_cast<int>(chrono::duration_cast<chrono::milliseconds>(stats.timeElapsed).count());
        int timePerPiece = 1000 / pps;

        if (timer >= millisec / timePerPiece) return;
        else timer++;
    }

    GameStats origStats = stats;
    bool useHold = false;
    int move = -1;
    RotateState rotation = INITIAL;

    currentBlock.ResetPosition();
    FindBestMove(useHold, move, rotation);

    stats = origStats;

    if (useHold)
        HoldBlock();

    currentBlock.ResetPosition();
    MakeMove(rotation, move);
    stats.droppedBlockCount++;
}

void TetrisHeurAI::Draw(const string& customTitle, const string& customData, const string& customSubData)
{
    renderer.UpdateScreenSize();

    renderer.DrawHoldBox(holdBlock);
    renderer.DrawQueueColumn(currentBag);

    if (!gameOver) renderer.DrawBoard(currentBlock, GetHardDropPos(), board);
    else renderer.DrawGameOver(board);

    renderer.DrawStats();
    renderer.DrawMessages();
    if (customTitle != "")
        renderer.DrawCustomStats(4, customTitle, customData, customSubData);
}

void TetrisHeurAI::UpdateHeuristics(HeuristicsWeights newWeights)
{
    weights = newWeights;
}

void TetrisHeurAI::SetPPS(float pps)
{
    this->pps = pps;
}

void TetrisHeurAI::NewGame()
{
    TetrisCore::NewGame();
    timer = 0;
}

void TetrisHeurAI::FindBestMove(bool& useHold, int& bestMove, RotateState& bestRotation)
{
    double bestRewardNoHold = -numeric_limits<float>::infinity();
    double bestRewardHold = -numeric_limits<float>::infinity();
    int bestMoveNoHold = -1;
    int bestMoveHold = -1;
    RotateState bestRotationNoHold = INITIAL;
    RotateState bestRotationHold = INITIAL;

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
        // 2 piece lookahead, so it may become redundant => Pull from cached result
        if (cacheExists)
        {
            bestRewardNoHold = cachedReward;
            bestMoveNoHold = cachedMove;
            bestRotationNoHold = cachedRotation;
        }
        else TryMoves(currentBlock, nextBlock, bestRewardNoHold, bestMoveNoHold, bestRotationNoHold);

        if (nextBlock != secondNextBlock)
        {
            TryMoves(nextBlock, secondNextBlock, bestRewardHold, bestMoveHold, bestRotationHold);
            
            cacheExists = true;
            cachedReward = bestRewardHold;
            cachedMove = bestMoveHold;
            cachedRotation = bestRotationHold;
        }
    }
    else
    {
        TryMoves(currentBlock, nextBlock, bestRewardNoHold, bestMoveNoHold, bestRotationNoHold);
        if (currentBlock != holdBlock)
            TryMoves(holdBlock, nextBlock, bestRewardHold, bestMoveHold, bestRotationHold);
    }

    if (bestRewardNoHold > bestRewardHold)
    {
        useHold = false;
        bestMove = bestMoveNoHold;
        bestRotation = bestRotationNoHold;
    }
    else
    {
        useHold = true;
        bestMove = bestMoveHold;
        bestRotation = bestRotationHold;
    }
}

double TetrisHeurAI::SimulateMove(Block& block, RotateState s, int posX)
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

void TetrisHeurAI::TryMoves(Block& firstBlock, Block& secondBlock, double& bestReward, int& bestMove, RotateState& bestRotation)
{
    BlockType firstType = firstBlock.GetType();
    BlockType secondType = secondBlock.GetType();

    double reward = -numeric_limits<float>::infinity();
    int move = -1;
    RotateState rotation = INITIAL;

    for (size_t i = 0; i < uniqueRotations[firstType]; ++i)
    {
        const RotateState tryRotation = (RotateState)i;
        Board orig1 = board;

        for (const auto& tryPosX : ParseMove(firstType, tryRotation))
        {
            double firstReward = SimulateMove(firstBlock, tryRotation, tryPosX);

            for (size_t j = 0; j < uniqueRotations[secondType]; ++j)
            {
                const RotateState tryRotation2 = (RotateState)i;
                Board orig2 = board;

                for (const auto& tryPosX2 : ParseMove(secondType, tryRotation2))
                {
                    double secondReward = SimulateMove(secondBlock, tryRotation2, tryPosX2);

                    if (firstReward + secondReward > reward)
                    {
                        reward = firstReward + secondReward;
                        move = tryPosX;
                        rotation = tryRotation;
                    }

                    board = orig2;
                    secondBlock.ResetPosition();
                }
            }

            board = orig1;
            firstBlock.ResetPosition();
        }
    }

    bestReward = reward;
    bestMove = move;
    bestRotation = rotation;
}
