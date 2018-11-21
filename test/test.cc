#include <vector>
#include <iostream>
#include <cassert>

#include "GeneticAlgorithm.h"
#include "BitVector.h"
#include "Individual.h"

using namespace panga;

struct TestUserData {
    size_t currentIndividual;
    size_t populationSize;
    size_t currentGeneration;
    size_t totalGenerations;
    size_t targetBitCount;
    BitVector targetBits;
    std::vector<double> generationalAverageScore;
    std::vector<double> generationalMinimumScore;
    std::vector<double> generationalStandardDeviation;
    std::vector<double> generationalPopulationDiversity;
};

double TestObjective(Individual* individual, void* userData) {
    TestUserData* testUserData = static_cast<TestUserData*>(userData);
    size_t failBits = testUserData->targetBits.HammingDistance(individual);

    std::cout << "Individual " << testUserData->currentGeneration << ":" << testUserData->currentIndividual << " => Error " << failBits << std::endl;

    testUserData->currentIndividual++;

    return (double) failBits;
}

int main(int argc, char** argv) {
    Genome genome;
    GeneticAlgorithm ga;
    TestUserData userData;

    genome.AddBooleanGenes(8);

    ga.SetGenome(&genome);
    ga.SetPopulationSize(100);
    ga.SetTotalGenerations(100);
    ga.SetMutationRate(0.05);
    ga.SetCrossoverRate(0.95);
    ga.SetEliteCount(1);
    ga.SetMutatedEliteCount(0);
    ga.SetMutatedEliteMutationRate(0.5);
    ga.SetFitnessFunction(TestObjective);
    ga.SetMutationRateSchedule(MutationRateSchedule::Deterministic);
    ga.SetCrossoverType(CrossoverType::TwoPoint);
    ga.SetMutatorType(MutatorType::Flip);
    ga.SetSelectorType(SelectorType::RouletteWheel);
    ga.SetAllowSameParentCouples(true);
    ga.SetUserData(&userData);
    ga.Initialize();

    userData.currentIndividual = 0;
    userData.currentGeneration = 0;
    userData.populationSize = ga.GetPopulationSize();
    userData.totalGenerations = ga.GetTotalGenerations();
    userData.targetBits.SetBitCount(genome.BitsRequired());
    userData.generationalAverageScore.resize(ga.GetTotalGenerations() + 1);
    userData.generationalMinimumScore.resize(ga.GetTotalGenerations() + 1);
    userData.generationalStandardDeviation.resize(ga.GetTotalGenerations() + 1);
    userData.generationalPopulationDiversity.resize(ga.GetTotalGenerations() + 1);

    for (userData.currentGeneration = 0; userData.currentGeneration < userData.totalGenerations; userData.currentGeneration++) {
        userData.currentIndividual = 0;

        std::cout << "Generation " << userData.currentGeneration
                  << " => avg: " << ga.GetAverageScore()
                  << " min: " << ga.GetMinScore()
                  << " stdev: " << ga.GetScoreStdev()
                  << " popdiv: " << ga.GetPopulationDiversity()
                  << std::endl;

        userData.generationalAverageScore[userData.currentGeneration] = ga.GetAverageScore();
        userData.generationalMinimumScore[userData.currentGeneration] = ga.GetMinScore();
        userData.generationalStandardDeviation[userData.currentGeneration] = ga.GetScoreStdev();
        userData.generationalPopulationDiversity[userData.currentGeneration] = ga.GetPopulationDiversity();

        ga.Step();
    }

    return 0;
}
