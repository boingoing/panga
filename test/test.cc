#include <vector>
#include <iostream>
#include <cassert>
#include <cstring>

#include "GeneticAlgorithm.h"
#include "BitVector.h"
#include "Individual.h"

using namespace panga;

struct TestUserData {
    size_t currentIndividual;
    size_t currentGeneration;
    BitVector targetBits;
};

double TestObjective(Individual* individual, void* userData) {
    TestUserData* testUserData = static_cast<TestUserData*>(userData);
    size_t failBits = testUserData->targetBits.HammingDistance(individual);

    //std::cout << "Individual " << testUserData->currentGeneration << ":" << testUserData->currentIndividual << " => Error " << failBits << std::endl;

    testUserData->currentIndividual++;

    return (double) failBits;
}

int main(int argc, const char** argv) {
    Genome genome;
    GeneticAlgorithm ga;
    TestUserData userData;

    if (argc > 1) {
        size_t len = strlen(argv[1]);
        userData.targetBits.FromString(argv[1], len);
    } else {
        userData.targetBits.SetBitCount(2000);
    }

    genome.AddBooleanGenes(userData.targetBits.GetBitCount());

    ga.SetGenome(&genome);
    ga.SetPopulationSize(100);
    ga.SetTotalGenerations(100);
    ga.SetMutationRate(0.0005);
    ga.SetCrossoverRate(0.99);
    ga.SetEliteCount(1);
    ga.SetMutatedEliteCount(0);
    ga.SetMutatedEliteMutationRate(0.5);
    ga.SetFitnessFunction(TestObjective);
    ga.SetMutationRateSchedule(MutationRateSchedule::Deterministic);
    ga.SetCrossoverType(CrossoverType::Uniform);
    ga.SetMutatorType(MutatorType::Flip);
    ga.SetSelectorType(SelectorType::Tournament);
    ga.SetTournamentSize(5);
    ga.SetAllowSameParentCouples(true);
    ga.SetUserData(&userData);
    ga.Initialize();

    double minScore = std::numeric_limits<double>::max();

    // minScore will fall below 1 when we've solved the problem
    while (minScore > 0.5) {
        // Pass the current generation and individual to the fitness function.
        // Reset current individual for this population
        userData.currentIndividual = 1;
        // Set this before we call Step (which calls the fitness function) so
        // add one to the current generation.
        userData.currentGeneration = ga.GetCurrentGeneration() + 1;

        ga.Step();

        std::cout << "Generation " << ga.GetCurrentGeneration()
                  << " => avg: " << ga.GetAverageScore()
                  << " min: " << ga.GetMinimumScore()
                  << " stdev: " << ga.GetScoreStandardDeviation()
                  << " popdiv: " << ga.GetPopulationDiversity()
                  << std::endl;

        if (ga.GetMinimumScore() < minScore) {
            minScore = ga.GetMinimumScore();
        }
    }

    return 0;
}
