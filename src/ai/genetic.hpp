#ifndef GENETIC_HPP
#define GENETIC_HPP

#include <atomic>
#include <chrono>
#include <format>
#include <functional>
#include <fstream>
#include <set>
#include <thread>

#include "ui/cli.hpp"
#include "heuristics.hpp"

constexpr int TRIALS_PER_GNOME = 20;
constexpr int POPULATION_SIZE = 100;
constexpr int MAX_LINES_PER_TRIAL = 3e5;
//constexpr int MAX_LINES_PER_TRIAL = 5e2;

constexpr int ELITISM_COUNT = POPULATION_SIZE * 5e-2;
constexpr float TOP_N_PARENTS = POPULATION_SIZE * 0.4;
constexpr float MUTATION_RATE = 3e-2;
constexpr double CONVERGENT_FITNESS = 1e-6;

constexpr int UPDATE_FREQ = 1e3;
constexpr int GUI_UPDATE_FREQ = 50; // milliseconds
constexpr int CLI_UPDATE_FREQ = 200; // milliseconds
constexpr int GUI_PER_CLI = CLI_UPDATE_FREQ / GUI_UPDATE_FREQ;

static uniform_real_distribution<float> geneGenerator(-1.0, 1.0);
static uniform_real_distribution<float> probabilityGen(0.0, 1.0);
static uniform_int_distribution<int> crossoverPointGen(0, 8);
static random_device rd;
static mt19937 rng(rd());


class Individual
{
public:
    Individual();
    Individual(HeuristicsWeights chromosome);
    Individual(HeuristicsWeights chromosome, double fitness);

    int currentTrial = 0;
    double totalLines = 0.0;
    double totalScore = 0.0;
    double currentLines = 0.0;
    double currentScore = 0.0;
    bool isRunning = false;

    double fitness = numeric_limits<float>::infinity();
    HeuristicsWeights chromosome;

    void CalculateFitness(
        int index,
        TetrisHeurAI& game,
        function<void(const int)>Start,
        function<void(const int)>End,
        bool& quitSignal
    );
    Individual Mate(Individual& partner);
    const bool operator<(const Individual& compare);
};


class Trainer
{
public:
    Trainer();

    void StartTraining();
    bool ShouldStop();
    Individual& GetBestIndividual();
    void SetConfig(int numThreads, int maxGeneration);

    int generation = 0;
private:
    vector<Individual> population;
    vector<TetrisHeurAI> games;

    int numThreads;
    int maxGeneration;

    vector<thread> threads;
    atomic<int> indices;
    set<int> runningIndices;
    set<int> finishedIndices;

    mutex mtx;

    int observeIndex = -1;
    bool quitSignal = false;

    Individual bestIndividual;

    void SaveData();

    void CreateThreadQueue();
    void StartFitness();
    void MatingPress();
    void Print();
    void Render();

    void StartSignal(const int index);
    void EndSignal(const int index);
};

#endif /* GENETIC_HPP */
