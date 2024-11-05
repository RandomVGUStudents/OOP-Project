#ifndef ENV_HPP
#define ENV_HPP

#include <algorithm>
#include <array>
#include "core/tetris.hpp"
#include "ui/ui.hpp"

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
    float holeCount = -1;
    float aggrHeight = -1;
    float maxHeight = -1;
    float bumpiness = -1;
    float rowTransition = -1;
    float colTransition = -1;
    float multiWell = -1;
    float wellDepth = 1;
    float gameScore = 1;

    array<float*, 9> asArray()
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

//class TetrisEnv : public TetrisUI
class TetrisEnv : public TetrisCore
{
public:
    TetrisEnv() {};

protected:
    BoardHeuristics heuristics;
    HeuristicsWeights weights;

    void CalcHeuristics(); // hole, aggrHeight, maxHeight, bumpiness, colTransition, wellSum
    int CalcScore();
    int CalcReward();

    void ParseMove(const BlockType currentType, int i, RotateState& s, int& posX);
    void MakeMove(RotateState s, int posX);
};

#endif /* ENV_HPP */
