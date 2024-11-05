#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include "env.hpp"

class TetrisHeurAI : public TetrisEnv
{
public:
    TetrisHeurAI() : renderer(stats, {{ SCORE, TIME, LINESPEED, BLOCKCOUNT, NONE }}) {};

    void Update();
    void Draw();
    void UpdateHeuristics(HeuristicsWeights newWeights);

protected:
    TetrisRenderer renderer;

    void FindBestMove(bool& useHold, int& move);
    int SimulateMove(Block& block, RotateState s, int posX);
    void TryMoves(Block& firstBlock, Block& secondBlock, int& bestReward, int& bestMove);
};

#endif /* HEURISTICS_HPP */
