#include "genetic.hpp"

Individual::Individual()
    : chromosome({
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng),
    geneGenerator(rng)})
{}

Individual::Individual(HeuristicsWeights chromosome)
    : chromosome(chromosome)
{}

Individual::Individual(HeuristicsWeights chromosome, double fitness)
    : chromosome(chromosome)
    , fitness(fitness)
{}

Individual Individual::Mate(Individual& partner)
{
    HeuristicsWeights childChromosome;
    int crossoverPoint = crossoverPointGen(rng);

    auto childGenes = childChromosome.asArray();
    auto parentGenes = chromosome.asArray();
    auto partnerGenes = partner.chromosome.asArray();

    for (size_t i = 0; i < childGenes.size(); ++i)
        if (probabilityGen(rng) < MUTATION_RATE) *childGenes.at(i) = geneGenerator(rng);
        else childGenes.at(i) = (i < crossoverPoint) ? parentGenes.at(i) : partnerGenes.at(i);

    return Individual(childChromosome);
}

bool Individual::operator<(const Individual& compare)
{
    return fitness < compare.fitness;
}

void Individual::CalculateFitness(TetrisHeurAI& game, int index, bool render)
{
    if (fitness != numeric_limits<float>::infinity()) return;

    isRunning = true;

    game.UpdateHeuristics(chromosome);
    game.NewGame();

    while (currentTrial < TRIALS_PER_GNOME)
    {
        game.Update();

        if (render)
        {
            gameWindow.BeginDrawing();
            gameWindow.ClearBackground();
            game.Draw(
                "individual no.",
                to_string(index),
                format(" run {}/{}", currentTrial, TRIALS_PER_GNOME)
            );
            gameWindow.EndDrawing();
        }

        if (game.stats.droppedBlockCount % UPDATE_FREQ == 0)
            cout << "Blocks count: " << game.stats.droppedBlockCount << " (PPS: "
                << format("{:.2f}/s", game.stats.droppedBlockCount / game.stats.timeElapsed.count())
                << ")" << endl;

        if (game.IsOver() || game.stats.clearedLineCount > MAX_LINES_PER_TRIAL)
        {
            currentTrial++;
            totalLines += game.stats.clearedLineCount;
            totalScore += game.stats.score;

            cout << "Individual no. " << index << " (" << currentTrial << "/" << TRIALS_PER_GNOME << ")" << endl;
            cout << "Line cleared: " << game.stats.clearedLineCount << endl;
            cout << "Blocks count: " << game.stats.droppedBlockCount << " (PPS: "
                << format("{:.2f}/s", game.stats.droppedBlockCount / game.stats.timeElapsed.count())
                << ")" << endl << endl;
            game.NewGame();
        }
    }

    fitness = TRIALS_PER_GNOME / (totalLines + totalScore * 0.01);
}


Trainer::Trainer()
    : games(POPULATION_SIZE)
    , render(false)
{
    LoadGeneration();
}

void Trainer::LoadGeneration(int gen)
{
    if (gen == -1)
    {
        std::regex pattern(R"(gen_(\d+)\.bin)");

        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
        {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::smatch match;

                if (std::regex_match(filename, match, pattern)) {
                    int number = std::stoi(match[1].str());
                    if (number > gen)
                        gen = number;
                }
            }
        }

        if (gen == -1) gen = 0;
    }

    ifstream f("gen_" + to_string(gen) + ".bin", ios::binary);
    if (f.is_open())
    {
        f.read(reinterpret_cast<char*>(&generation), sizeof(generation));

        for (size_t i = 0; i < POPULATION_SIZE; ++i)
        {
            HeuristicsWeights chromosome;
            double fitness;

            for (int j = 0; j < 9; ++j)
                f.read(reinterpret_cast<char*>(chromosome.asArray()[j]), sizeof(double));
            f.read(reinterpret_cast<char*>(&fitness), sizeof(double));

            population.push_back(Individual(chromosome, fitness));
        }

        sort(population.begin(), population.end());
        bestIndividual = population.front();

        f.close();
    }
    else population = vector<Individual>(POPULATION_SIZE);
}

void Trainer::StartTraining(bool render)
{
    for (size_t i = 0; i < POPULATION_SIZE; ++i)
        population.at(i).CalculateFitness(games.at(i), i, render);

    MatingPress();
    SaveData();
}

bool Trainer::ShouldStop()
{
    return bestIndividual.fitness < CONVERGENT_FITNESS || generation > MAX_GENERATION;
}

Individual& Trainer::GetBestIndividual()
{
    return bestIndividual;
}

void Trainer::SaveData()
{
    ofstream f("gen_" + to_string(generation) + ".bin", ios::binary);
    if (!f.is_open()) return;

    f.write(reinterpret_cast<const char*>(&generation), sizeof(generation));

    for (auto& individual : population) {
        auto chromosomeArray = individual.chromosome.asArray();
        for (int i = 0; i < 9; ++i) {
            f.write(reinterpret_cast<const char*>(chromosomeArray[i]), sizeof(double));
        }
        f.write(reinterpret_cast<const char*>(&individual.fitness), sizeof(individual.fitness));
    }

    f.close();
}

void Trainer::MatingPress()
{
    sort(population.begin(), population.end());
    bestIndividual = population.front();

    if (bestIndividual.fitness < CONVERGENT_FITNESS) return;

    vector<Individual> offspring;
    for (size_t i = 0; i < ELITISM_COUNT; ++i)
        offspring.push_back(std::move(population.at(i)));

    for (size_t i = ELITISM_COUNT; i < POPULATION_SIZE; ++i)
    {
        Individual dad = population.at(int(TOP_N_PARENTS * probabilityGen(rng)));
        Individual mom = population.at(int(TOP_N_PARENTS * probabilityGen(rng)));
        Individual kid = dad.Mate(mom);
        offspring.push_back(kid);
    }

    population = offspring;
    generation++;
}
