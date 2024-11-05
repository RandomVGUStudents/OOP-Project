#include "genetic.hpp"

Individual::Individual() : chromosome({
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng)
}) {}

Individual::Individual(HeuristicsWeights chromosome) : chromosome(chromosome) {}

Individual Individual::mate(Individual& partner)
{
    HeuristicsWeights childChromosome;
    int crossoverPoint = crossoverPointGen(rng);

    for (size_t i = 0; i < 9; ++i)
    {
        if (probabilityGen(rng) < MUTATION_RATE)
            *childChromosome.asArray().at(i) = geneGenerator(rng);
        else
        {
            if (i < crossoverPoint)
                childChromosome.asArray().at(i) = chromosome.asArray().at(i);
            else
                childChromosome.asArray().at(i) = partner.chromosome.asArray().at(i);
        }
    }

    return Individual(childChromosome);
}

const bool Individual::operator<(const Individual& compare)
{
    return fitness < compare.fitness;
}

void Individual::CalculateFitness()
{
    TetrisHeurAI game;
    game.UpdateHeuristics(chromosome);

    double totalLineCleared = 0.0;
    double totalScore = 0.0;

    for (int trials = 0; trials < TRIALS_PER_GNOME && totalLineCleared < MAX_LINES_PER_TRIAL;)
    {
        gameWindow.BeginDrawing();
        gameWindow.ClearBackground();

        game.Update();
        game.Draw();

        if (game.IsOver())
        {
            trials++;
            totalLineCleared += game.stats.clearedLineCount;
            totalScore += game.stats.score;

            game.NewGame();
        }

        gameWindow.EndDrawing();
    }

    fitness = TRIALS_PER_GNOME / (totalLineCleared + totalScore * 0.01);
}
