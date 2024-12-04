#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include <chrono>
#include <fstream>
#include "ui/renderer.hpp"
#include "env.hpp"

class TetrisHeurAI : public TetrisEnv
{
public:
    TetrisHeurAI();

    void Update();
    void Draw(const string& customTitle="", const string& customData="", const string& customSubData="");
    void UpdateHeuristics(HeuristicsWeights newWeights);
    void SetPPS(float pps);
    
    void NewGame() override;

protected:
    float pps;
    int timer;

    TetrisRenderer renderer;

    void FindBestMove(bool& useHold, int& bestMove, RotateState& bestRotation);
    double SimulateMove(Block& block, RotateState s, int posX);
    void TryMoves(Block& firstBlock, Block& secondBlock, double& bestReward, int& bestMove, RotateState& bestRotation);
};

#endif /* HEURISTICS_HPP */
