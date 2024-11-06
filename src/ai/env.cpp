#include "env.hpp"

void TetrisEnv::CalcHeuristics()
{
    heuristics = BoardHeuristics();
    int lastLineHeight = 0;

    for (size_t i = 0; i < BOARD_WIDTH; ++i)                                                                                                           
    {                                                                                                                                                  
        bool blockFound = false;                                                                                                                       
        bool cell = false;

        for (size_t j = 2; j < BOARD_HEIGHT; ++j)                                                                                                      
        {
            bool cellFilled = board.GetCell(i, j) != EMPTY; 
            if (cellFilled && !blockFound)
            {
                blockFound = true;                                                                                                                     
                int lineHeight = BOARD_HEIGHT - j;
                if (i == 0)
                    lastLineHeight = lineHeight;

                heuristics.aggrHeight += lineHeight;
                heuristics.maxHeight = max(lineHeight, heuristics.maxHeight);
                heuristics.bumpiness += abs(lineHeight - lastLineHeight);

                lastLineHeight = lineHeight;
            }
            else if (!cellFilled && blockFound)                                                                                                                       
            {
                heuristics.holeCount++;                                                                                                                               
                if (cell)
                {
                    heuristics.colTransition++;
                    cell = cellFilled;
                }
            }
            else if (cellFilled && blockFound && !cell)
            {
                heuristics.colTransition++;
                cell = cellFilled;
            }
            else if (j == BOARD_HEIGHT - 1)
            {
                heuristics.bumpiness += lastLineHeight;
                lastLineHeight = 0;
            }

            cell = cellFilled;
        }

        // Check for a well
        Block wellTester;
        bool isWell = false;
        int wellDepth = 0;
        int wellRows = BOARD_HEIGHT - lastLineHeight - wellDepth - 1;

        while (board.CheckFit(int(i), wellRows, wellTester)
            && !board.CheckFit(int(i) - 1, wellRows, wellTester)
            && !board.CheckFit(int(i) + 1, wellRows, wellTester))
        {
            if (++wellDepth >= 3)
                isWell = true;
            wellRows = BOARD_HEIGHT - lastLineHeight - wellDepth - 1;
        }

        if (isWell)
        {
            heuristics.wellDepth = max(wellDepth, heuristics.wellDepth);
            heuristics.additionalWell++;
        }
    }                                                                                                                                                  

    // Row transition
    for (size_t i = BOARD_HEIGHT - heuristics.maxHeight; i < BOARD_HEIGHT; ++i)
    {
        bool cell = board.GetCell(0, i) == EMPTY; 
        for (size_t j = 1; j < BOARD_WIDTH; ++j)
        {
            bool thisCell = board.GetCell(j, i) == EMPTY;
            if (cell != thisCell)
            {
                heuristics.rowTransition++;
                cell = thisCell;
            }
        }
    }
}

int TetrisEnv::CalcScore()
{
    int clearedLine = board.CheckFullRow();
    int lastScore = stats.score;

    if (clearedLine == 0)
    {
        stats.comboCount = -1;
        return 0;
    }

    stats.comboCount++;
    stats.clearedLineCount += clearedLine;
    stats.score += stats.comboCount * 50 * stats.level;

    int baseScore = 0;
    switch (clearedLine)
    {
        case 1: baseScore = 100; stats.b2bChain = -1; break;
        case 2: baseScore = 300; stats.b2bChain = -1; break;
        case 3: baseScore = 500; stats.b2bChain = -1; break;
        case 4: baseScore = 800; stats.tetrisCount++; stats.b2bChain++; break;
    }

    if (stats.b2bChain > 0)
        baseScore *= 1.5;

    if (board.CheckFullClear())
    {
        stats.fullClearCount++;
        switch (clearedLine)
        {
            case 1: baseScore += 800; break;
            case 2: baseScore += 1200; break;
            case 3: baseScore += 1800; break;
            case 4: baseScore += (stats.b2bChain > 0) ? 3200 : 2000; break;
        }
    }

    stats.score += baseScore * stats.level;
    return stats.score - lastScore;
}

int TetrisEnv::CalcReward()
{
    CalcHeuristics();
    return weights.holeCount * heuristics.holeCount
    + weights.aggrHeight * heuristics.aggrHeight
    + weights.maxHeight * heuristics.maxHeight
    + weights.bumpiness * heuristics.bumpiness
    + weights.rowTransition * heuristics.rowTransition
    + weights.colTransition * heuristics.colTransition
    + weights.wellDepth * heuristics.wellDepth
    + weights.multiWell * heuristics.additionalWell
    + weights.gameScore * CalcScore();
}

void TetrisEnv::ParseMove(const BlockType currentType, int i, RotateState& tryRotation, int& tryPosX)
{
    switch (currentType)
    { // Hardcode brrr
        case I:
            if (i < 7)
            {
                tryRotation = INITIAL;
                tryPosX = i;
            }
            else
            {
                tryRotation = LEFT;
                tryPosX = i - 7 - 1; // Offsetting I piece in the second row of its grid
            }
            break;

        case J:
        case L:
        case T:
            if (i < 17)
            {
                if (i < 8)
                {
                    tryRotation = INITIAL;
                    tryPosX = i;
                }
                else
                {
                    tryRotation = LEFT;
                    tryPosX = i - 8;
                }
            }
            else
            {
                if (i < 25)
                {
                    tryRotation = DOWN;
                    tryPosX = i - 17;
                }
                else
                {
                    tryRotation = RIGHT;
                    tryPosX = i - 25 - 1;
                }
            }
            break;

        case O:
            tryRotation = INITIAL;
            tryPosX = i;
            break;

        case S:
        case Z:
            if (i < 8)
            {
                tryRotation = INITIAL;
                tryPosX = i;
            }
            else
            {
                tryRotation = LEFT;
                tryPosX = i - 8;
            }
            break;

    }
}

void TetrisEnv::MakeMove(RotateState s, int posX)
{
    currentBlock.Rotate(s);
    currentBlock.Move(posX, 0);
    currentBlock.Move(0, GetHardDropPos());

    UpdateBoard();
    CalcScore();
}
