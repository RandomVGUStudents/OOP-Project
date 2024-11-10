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

const bool Individual::operator<(const Individual& compare)
{
    return fitness < compare.fitness;
}

void Individual::CalculateFitness(
    int index,
    TetrisHeurAI& game,
    function<void(const int)>Start,
    function<void(const int)>End,
    bool& quitSignal
)
{
    if (fitness != numeric_limits<float>::infinity())
    {
        End(index);
        return;
    }

    isRunning = true;
    Start(index);

    game.UpdateHeuristics(chromosome);

    while (currentTrial < TRIALS_PER_GNOME && !quitSignal)
    {
        game.Update();

        if (game.IsOver() || game.stats.clearedLineCount > MAX_LINES_PER_TRIAL)
        {
            currentTrial++;
            totalLines += game.stats.clearedLineCount;
            totalScore += game.stats.score;

            game.NewGame();
        }
    }

    if (!quitSignal) fitness = TRIALS_PER_GNOME / (totalLines + totalScore * 0.01);
    isRunning = false;
    End(index);
}


Trainer::Trainer()
    : games(POPULATION_SIZE)
{
    ifstream f(DATAFILE, ios::binary);
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

void Trainer::StartTraining()
{
    quitSignal = false;
    CreateThreadQueue();

    int counter = 0;
    while (finishedIndices.size() < POPULATION_SIZE)
    {
        if (raylib::Keyboard::IsKeyPressed(KEY_ESCAPE) || gameWindow.ShouldClose())
        {
            quitSignal = true;
            for (auto& thread : threads) if (thread.joinable()) thread.join();
            return;
        }

        this_thread::sleep_for(chrono::milliseconds(GUI_UPDATE_FREQ));
        Render();
        counter++;

        if (counter == GUI_PER_CLI)
        {
            Print();
            counter = 0;
        }
    }

    for (auto& thread : threads) if (thread.joinable()) thread.join();

    MatingPress();
    SaveData();
}

bool Trainer::ShouldStop()
{
    bool stopCuzQuitSignal = quitSignal;
    quitSignal = false;
    return bestIndividual.fitness < CONVERGENT_FITNESS
    || generation > maxGeneration
    || stopCuzQuitSignal;
}

Individual& Trainer::GetBestIndividual()
{
    return bestIndividual;
}

void Trainer::SetConfig(int thread, int generation)
{
    numThreads = thread;
    maxGeneration = generation;
}

void Trainer::SaveData()
{
    ofstream f(DATAFILE, ios::binary);
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

void Trainer::CreateThreadQueue()
{
    indices = POPULATION_SIZE - 1;
    runningIndices.clear();
    finishedIndices.clear();

    for (size_t i = 0; i < numThreads; ++i)
        threads.emplace_back(&Trainer::StartFitness, this);
}

void Trainer::StartFitness()
{
    while(indices >= 0)
    {
        int index = indices.fetch_sub(1);

        std::function<void(const int)> startSignalFunc = [this](const int value) {
            this->StartSignal(value);
        };
        std::function<void(const int)> endSignalFunc = [this](const int value) {
            this->EndSignal(value);
        };

        population[index].CalculateFitness(
            index,
            games[index], 
            startSignalFunc,
            endSignalFunc,
            quitSignal
        );
    }
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

void Trainer::Print()
{
    cout << CLEAR_SCREEN;

    double overallProgress = (POPULATION_SIZE * generation + finishedIndices.size()) * 100.0 / 
        (POPULATION_SIZE * (maxGeneration + 1));
    cout << createBox(
        BOLD + "Overall Training Progress" + RESET,
        CLI_GREEN + createProgressBar(overallProgress) + RESET
    ) << endl << endl;

    double genProgress = finishedIndices.size() * 100.0 / POPULATION_SIZE;
    stringstream ss;
    ss << BOLD << "Current Generation: " << generation << RESET;
    ss << finishedIndices.size() << "/" << POPULATION_SIZE;

    cout << createBox(ss.str(), CLI_BLUE + createProgressBar(genProgress) + RESET)
        << endl << endl;

    if (generation > 0)
    {
        stringstream fitnessStr;
        fitnessStr << fixed << setprecision(10) << bestIndividual.fitness;
        cout << createBox("Best Fitness", fitnessStr.str()) << endl << endl;
    }

    cout << BOLD << "Currently Running Individuals" << RESET << endl
        << string(50, '-') << endl;

    for (const auto& index : runningIndices)
    {
        const auto& individual = population[index];
        int trial = individual.currentTrial;
        double trialProgress = trial * 100.0 / TRIALS_PER_GNOME;

        cout << left << (index == observeIndex ? CLI_RED : CLI_YELLOW)
            << "Individual " << setw(3) << index << RESET

            << createProgressBar(trialProgress, 20) << endl

            << "Trial " << setw(10) 
            << (to_string(trial) + "/" + to_string(TRIALS_PER_GNOME))
            << "Lines: " << setw(10) 
            << static_cast<int>(individual.totalLines)
            << "Score: " 
            << static_cast<int>(individual.totalScore)
            << endl;

        cout << string(50, '-') << endl;
    }
}

void Trainer::Render()
{
    if (observeIndex == -1 || !population.at(observeIndex).isRunning)
    {
        int random = static_cast<int>(probabilityGen(rng) * runningIndices.size());
        auto it = runningIndices.begin();
        advance(it, random);

        observeIndex = *it;
    }

    gameWindow.BeginDrawing();
    gameWindow.ClearBackground();

    const auto& individual = population[observeIndex];
    games[observeIndex].Draw(
        "individual no.",
        to_string(observeIndex),
        format(" run {}/{}", individual.currentTrial, TRIALS_PER_GNOME)
    );

    gameWindow.EndDrawing();
}

void Trainer::StartSignal(const int index)
{
    lock_guard<mutex> lock(mtx);
    runningIndices.insert(index);
}

void Trainer::EndSignal(const int index)
{
    lock_guard<mutex> lock(mtx);
    runningIndices.erase(index);
    finishedIndices.insert(index);
}
