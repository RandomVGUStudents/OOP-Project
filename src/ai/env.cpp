#include "env.hpp"

void TetrisEnv::CalcHeuristics()
{
    heuristics = BoardHeuristics();
    int lastColHeight = 0;

    for (size_t i = 0; i < BOARD_WIDTH; ++i)                                                                                                           
    {                                                                                                                                                  
        bool foundFirstFilledCell = false;                                                                                                                       
        bool prevCell = false;
        int colHeight = 0;
        int wellDepth = 0;

        // Scanning from top to bottom, excluding first 2 rows where pieces spawn
        for (size_t j = 2; j < BOARD_HEIGHT; ++j)                                                                                                      
        {
            bool cellIsFilled = board.GetCell(i, j) != EMPTY; 

            // Encounter a filled cell for the first time
            if (cellIsFilled && !foundFirstFilledCell)
            {
                foundFirstFilledCell = true;                                                                                                                     
                colHeight = BOARD_HEIGHT - j;

                if (i == 0) lastColHeight = colHeight;

                heuristics.aggrHeight += colHeight;
                heuristics.maxHeight = max(colHeight, heuristics.maxHeight);
                heuristics.bumpiness += abs(colHeight - lastColHeight);

                lastColHeight = colHeight;
            }

            // Encounter a blank cell below a filled cell
            // 1. A hole
            // 2. If the cell right above is also filled, a column transition
            else if (!cellIsFilled && foundFirstFilledCell)                                                                                                                       
            {
                heuristics.holeCount++;                                                                                                                               
                if (prevCell)
                {
                    heuristics.colTransition++;
                    prevCell = cellIsFilled;
                }
            }

            // Encounter a filled cell, not the first filled one in the column
            // and the cell above is blank, a column transition
            else if (cellIsFilled && foundFirstFilledCell && !prevCell)
            {
                heuristics.colTransition++;
                prevCell = cellIsFilled;
            }

            // Reached the bottom while no flag is set, a whole empty column
            else if (j == BOARD_HEIGHT - 1)
            {
                heuristics.bumpiness += lastColHeight;
                lastColHeight = 0;
            }

            prevCell = cellIsFilled;
        }

        // Check for a well
        Block wellTester;
        bool isWell = false;
        int wellRows = BOARD_HEIGHT - lastColHeight - wellDepth - 1;

        while (board.CheckFit(int(i), wellRows, wellTester)
            && !board.CheckFit(int(i) - 1, wellRows, wellTester)
            && !board.CheckFit(int(i) + 1, wellRows, wellTester))
        {
            if (++wellDepth >= 3) isWell = true;
            wellRows = BOARD_HEIGHT - lastColHeight - wellDepth - 1;
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
        bool prevCell = board.GetCell(0, i) != EMPTY; 
        for (size_t j = 1; j < BOARD_WIDTH; ++j)
        {
            bool thisCell = board.GetCell(j, i) != EMPTY;
            if (prevCell != thisCell)
            {
                heuristics.rowTransition++;
                prevCell = thisCell;
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

double TetrisEnv::CalcReward()
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

void TetrisEnv::MakeMove(RotateState s, int posX)
{
    currentBlock.Rotate(s);
    currentBlock.Move(posX, 0);
    currentBlock.Move(0, GetHardDropPos());

    UpdateBoard();
    CalcScore();
}
