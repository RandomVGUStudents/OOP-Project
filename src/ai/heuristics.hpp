#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include <chrono>
#include <fstream>
#include "ui/renderer.hpp"
#include "env.hpp"

constexpr string DATAFILE = "data.bin";

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
    TetrisRenderer renderer;
    float pps;
    int timer;

    // Cached result
    bool cacheExists;
    double cachedReward;
    int cachedMove;
    RotateState cachedRotation;

    void FindBestMove(bool& useHold, int& bestMove, RotateState& bestRotation);
    double SimulateMove(Block& block, RotateState s, int posX);
    void TryMoves(Block& firstBlock, Block& secondBlock, double& bestReward, int& bestMove, RotateState& bestRotation);
};

#endif /* HEURISTICS_HPP */
