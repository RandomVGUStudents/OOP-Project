#include <algorithm>
#include <array>
#include <iostream>
#include "ai/genetic.hpp"
raylib::Window gameWindow(1920, 1080, "Tetr.io Clone");
raylib::Texture minoTexture("./sprite.png");
raylib::Font font("./hun2.ttf", 80);

int main()
{
    gameWindow.SetFullscreen(true);
    gameWindow.SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    gameWindow.SetConfigFlags(FLAG_MSAA_4X_HINT);
    //gameWindow.SetTargetFPS(10);
    font.GetTexture().GenMipmaps();
    font.GetTexture().SetFilter(TEXTURE_FILTER_BILINEAR);

    int generation = 0;
    bool stop = false;
    array<Individual, POPULATION_SIZE> population;

    while (!stop && !gameWindow.ShouldClose())
    {
        for (size_t i = 0; i < POPULATION_SIZE; ++i)
        {
            population.at(i).CalculateFitness();
            cout << "Individual " << i
                << " has fitness of "
                << population.at(i).fitness
                << endl;
        }
        sort(population.begin(), population.end());

        if (population.front().fitness < CONVERGENT_FITNESS)
        {
            stop = true;
            break;
        }

        array<Individual, POPULATION_SIZE> offspring;
        for (size_t i = 0; i < ELITISM_COUNT; ++i)
            offspring.at(i) = population.at(i);

        for (size_t i = ELITISM_COUNT; i < POPULATION_SIZE; ++i)
        {
            Individual dad = population.at(int(TOP_N_PARENTS * probabilityGen(rng)));
            Individual mom = population.at(int(TOP_N_PARENTS * probabilityGen(rng)));
            Individual kid = dad.mate(mom);
            offspring.at(i) = kid;
        }

        population = offspring;

        cout
            << "Generation: "
            << generation
            << ", Fitness: "
            << population[0].fitness
            << endl
            << "Weights: ";

        for (size_t i = 0; i < 9; ++i)
            cout << population.at(0).chromosome.asArray().at(i) << ", ";

        cout << endl;

        generation++;
    }

    gameWindow.Close();
    cout
        << "Generation: "
        << generation
        << ", Fitness: "
        << population[0].fitness
        << endl
        << "Weights: ";

    for (size_t i = 0; i < 9; ++i)
        cout << population.at(0).chromosome.asArray().at(i) << ", ";

    cout << endl;
}
