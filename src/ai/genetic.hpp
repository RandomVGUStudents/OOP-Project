#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "heuristics.hpp"

constexpr int TRIALS_PER_GNOME = 20;
constexpr int POPULATION_SIZE = 100;
constexpr int MAX_LINES_PER_TRIAL = 1e6;

constexpr int ELITISM_COUNT = POPULATION_SIZE * 5e-2;
constexpr float TOP_N_PARENTS = POPULATION_SIZE * 0.4;
constexpr float MUTATION_RATE = 3e-2;
constexpr double CONVERGENT_FITNESS = 1e-6;

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

    double fitness;
    HeuristicsWeights chromosome;

    void CalculateFitness();
    Individual mate(Individual& partner);
    const bool operator<(const Individual& compare);

    static HeuristicsWeights createChromosome();
};

#endif /* GENETIC_HPP */
