#ifndef ENV_HPP
#define ENV_HPP

#include <algorithm>
#include <array>
#include "core/tetris.hpp"

constexpr array<int, BLOCK_TYPES + 1> placePositions = {{
    7 + 10,      // I piece: 7 horizontal pos, 10 vertical pos, 2 rotations
    (8 + 9) * 2, // J piece: 8 horizontal pos, 9 vertical pos, 4 rotations
    (8 + 9) * 2, // L piece: 8 horizontal pos, 9 vertical pos, 4 rotations
    9,           // O piece: 9 positions, 1 rotation
    8 + 9,       // S piece: 8 horizontal pos, 9 vertical pos, 2 rotations
    (8 + 9) * 2, // T piece: 8 horizontal pos, 9 vertical pos, 4 rotations
    8 + 9        // Z piece: 8 horizontal pos, 9 vertical pos, 2 rotations
}};

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
    int CalcReward();

    void ParseMove(const BlockType currentType, int i, RotateState& s, int& posX);
    void MakeMove(RotateState s, int posX);
};

#endif /* ENV_HPP */
