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

    void FindBestMove(bool& useHold, int& move);
    int SimulateMove(Block& block, RotateState s, int posX);
    void TryMoves(Block& firstBlock, Block& secondBlock, int& bestReward, int& bestMove);
};

#endif /* HEURISTICS_HPP */
