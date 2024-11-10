#ifndef ENV_HPP
#define ENV_HPP

#include <algorithm>
#include <array>
#include "core/common.hpp"
#include "core/tetris.hpp"

constexpr array<int, BLOCK_TYPES> uniqueRotations = {{
    2, // I piece: 7 horizontal pos, 10 vertical pos, 2 rotations
    4, // J piece: 8 horizontal pos, 9 vertical pos, 4 rotations
    4, // L piece: 8 horizontal pos, 9 vertical pos, 4 rotations
    1, // O piece: 9 positions, 1 rotation
    2, // S piece: 8 horizontal pos, 9 vertical pos, 2 rotations
    4, // T piece: 8 horizontal pos, 9 vertical pos, 4 rotations
    2  // Z piece: 8 horizontal pos, 9 vertical pos, 2 rotations
}};

constexpr vector<int> createVecRange(int start, int end)
{
    vector<int> vec;
    for (int i = start; i <= end; ++i) vec.push_back(i);
    return vec;
}

constexpr vector<int> ParseMove(const BlockType type, const RotateState state)
{
    switch (type)
    {
        case I:
            switch (state)
            {
                case INITIAL: return createVecRange(0, 6); break;
                case LEFT: return createVecRange(-1, 8); break;
                case DOWN: return createVecRange(0, 6); break;
                case RIGHT: return createVecRange(-2, 7); break;
            }
            break;

        case J:
        case L:
        case T:
            switch (state)
            {
                case INITIAL: return createVecRange(0, 7); break;
                case LEFT: return createVecRange(0, 8); break;
                case DOWN: return createVecRange(0, 7); break;
                case RIGHT: return createVecRange(0, 8); break;
            }
            break;

        case O:
            switch (state)
            {
                case INITIAL: return createVecRange(0, 8); break;
                case LEFT: return createVecRange(0, 8); break;
                case DOWN: return createVecRange(0, 8); break;
                case RIGHT: return createVecRange(0, 8); break;
            }
            break;

        case S:
        case Z:
            switch (state)
            {
                case INITIAL: return createVecRange(0, 8); break;
                case LEFT: return createVecRange(0, 8); break;
                case DOWN: return createVecRange(0, 8); break;
                case RIGHT: return createVecRange(0, 8); break;
            }
            break;

        case EMPTY: break;
    }
    return {};
}


struct BoardHeuristics
{
    int holeCount = 0;
    int aggrHeight = 0;
    int maxHeight = 0;
    int bumpiness = 0;
    int rowTransition = 0;
    int colTransition = 0;
    int wellDepth = 0;
    int additionalWell = -1;
};

struct HeuristicsWeights
{
    double holeCount = -1;
    double aggrHeight = -1;
    double maxHeight = -1;
    double bumpiness = -1;
    double rowTransition = -1;
    double colTransition = -1;
    double multiWell = -1;
    double wellDepth = 1;
    double gameScore = 1;

    array<double*, 9> asArray()
    {
        return {{
            &holeCount,
            &aggrHeight,
            &maxHeight,
            &bumpiness,
            &rowTransition,
            &colTransition,
            &multiWell,
            &wellDepth,
            &gameScore,
        }};
    }
};

class TetrisEnv : public TetrisCore
{
public:
    TetrisEnv() {};

protected:
    BoardHeuristics heuristics;
    HeuristicsWeights weights;

    void CalcHeuristics();
    int CalcScore();
    double CalcReward();

    void MakeMove(RotateState s, int posX);
};

#endif /* ENV_HPP */
