#ifndef GENETIC_HPP
#define GENETIC_HPP

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>

#include "heuristics.hpp"

constexpr int TRIALS_PER_GNOME = 20;
constexpr int POPULATION_SIZE = 100;
constexpr int MAX_LINES_PER_TRIAL = 3e5;
constexpr int MAX_GENERATION = 20;

constexpr int ELITISM_COUNT = POPULATION_SIZE * 5e-2;
constexpr float TOP_N_PARENTS = POPULATION_SIZE * 0.4;
constexpr float MUTATION_RATE = 3e-2;
constexpr double CONVERGENT_FITNESS = 1e-6;

constexpr int GUI_UPDATE_FREQ = 50; // milliseconds
constexpr int CLI_UPDATE_FREQ = 200; // milliseconds
constexpr int GUI_PER_CLI = CLI_UPDATE_FREQ / GUI_UPDATE_FREQ;
constexpr int UPDATE_FREQ = 50000; // 50k blocks per update

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
    bool isRunning = false;

    HeuristicsWeights chromosome;
    double fitness = numeric_limits<float>::infinity();

    void CalculateFitness(TetrisHeurAI& game, int index, bool render=false);
    Individual Mate(Individual& partner);
    bool operator<(const Individual& compare);
};


class Trainer
{
public:
    Trainer();

    void LoadGeneration(int generation=-1);
    void StartTraining(bool render);
    bool ShouldStop();
    Individual& GetBestIndividual();

    int generation = 0;
private:
    vector<Individual> population;
    vector<TetrisHeurAI> games;
    Individual bestIndividual;

    bool render;

    void SaveData();
    void MatingPress();
};

#endif /* GENETIC_HPP */
