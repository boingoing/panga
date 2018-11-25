//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <vector>
#include <iostream>
#include <cassert>
#include <cstring>
#include <sstream>

#include "GeneticAlgorithm.h"
#include "BitVector.h"
#include "Individual.h"

using namespace panga;

bool VerboseOutput = false;

#define AssertTrue(expr, msg) \
if (!(expr)) { \
    std::cerr << "Failure: " << msg << " (" << #expr << ")" << std::endl; \
    assert(expr); \
    return false; \
}

#define ReturnErrorIfFalse(expr) \
if (!(expr)) { \
    std::cerr << "Failure: " << #expr << std::endl; \
    return -1; \
}

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

bool TestSolveMatchingProblem(const char* target) {
    Genome genome;
    GeneticAlgorithm ga;
    TestUserData userData;

    if (target != nullptr) {
        size_t len = strlen(target);
        userData.targetBits.FromString(target, len);
    } else {
        userData.targetBits.SetBitCount(2000);
    }

    genome.AddBooleanGenes(userData.targetBits.GetBitCount());
    AssertTrue(genome.BitsRequired() == userData.targetBits.GetBitCount(), "Genome encodes correct number of bits");

    ga.SetGenome(&genome);
    ga.SetPopulationSize(100);
    ga.SetTotalGenerations(100);
    ga.SetMutationRate(0.0005);
    ga.SetCrossoverRate(0.99);
    ga.SetEliteCount(1);
    ga.SetMutatedEliteCount(0);
    ga.SetMutatedEliteMutationRate(0.5);
    ga.SetFitnessFunction(TestObjective);
    ga.SetMutationRateSchedule(GeneticAlgorithm::MutationRateSchedule::Proportional);
    ga.SetCrossoverType(GeneticAlgorithm::CrossoverType::Uniform);
    ga.SetMutatorType(GeneticAlgorithm::MutatorType::Flip);
    ga.SetSelectorType(GeneticAlgorithm::SelectorType::Tournament);
    ga.SetTournamentSize(5);
    ga.SetKPointCrossoverPointCount(5);
    ga.SetProportionalMutationBitCount(1);
    ga.SetAllowSameParentCouples(true);
    ga.SetUserData(&userData);
    ga.Initialize();

    // Minimum score will fall below 1 when we've solved the problem
    do {
        // Pass the current generation and individual to the fitness function.
        // Reset current individual for this population
        userData.currentIndividual = 1;
        // Set this before we call Step (which calls the fitness function) so
        // add one to the current generation.
        userData.currentGeneration = ga.GetCurrentGeneration() + 1;

        ga.Step();

        if (VerboseOutput) {
            std::cout << "Generation " << ga.GetCurrentGeneration()
                << " => avg: " << ga.GetAverageScore()
                << " min: " << ga.GetMinimumScore()
                << " stdev: " << ga.GetScoreStandardDeviation()
                << " popdiv: " << ga.GetPopulationDiversity()
                << std::endl;
        }
    } while (ga.GetMinimumScore() > 0.5);

    return true;
}

bool TestCrossoverGenes(size_t geneCount, size_t geneWidth) {
    Genome genome;
    for (size_t i = 0; i < geneCount; i++) {
        size_t geneIndex = genome.AddGene(geneWidth);
        AssertTrue(geneIndex == i, "Gene index");
    }
    AssertTrue(genome.BitsRequired() == geneWidth * geneCount, "Genes add up to the corect length");

    Individual left(&genome);
    Individual right(&genome);
    Individual offspring(&genome);
    AssertTrue(genome.BitsRequired() == left.GetBitCount(), "Individuals based on a genome have length eqaul to the genome bits required");

    // Alternate gene values between all 0s and all 1s
    for (size_t i = 0; i < geneCount; i++) {
        uint64_t leftValue = i % 2 ? std::numeric_limits<uint64_t>::max() : 0;
        uint64_t rightValue = i % 2 ? 0 : std::numeric_limits<uint64_t>::max();
        left.EncodeIntegerGene<uint64_t, false>(i, leftValue);
        right.EncodeIntegerGene<uint64_t, false>(i, rightValue);
    }

    RandomWrapper randomWrapper;

    // Perform crossover respecting gene boundaries.
    // This means we will take all the bits of a gene instead of cutting them up.
    Individual::UniformCrossover<false>(&left, &right, &offspring, &randomWrapper);

    for (size_t i = 0; i < geneCount; i++) {
        uint64_t value = offspring.DecodeIntegerGene<uint64_t, false>(i);
        uint64_t maxValue = std::numeric_limits<uint64_t>::max() >> (sizeof(uint64_t)*8 - geneWidth);
        AssertTrue(value == 0 || value == maxValue, "Decoded value should be all 1s or all 0s");
    }

    return true;
}

bool TestBitVectorToString(BitVector* bv, const char* expectedBinString, const char* expectedHexString) {
    char buf[2000];
    bv->ToStringHex(buf, sizeof(buf));
    AssertTrue(std::strcmp(buf, expectedHexString) == 0, "BitVector::ToStringHex produces expected result");
    std::stringstream sstr1;
    sstr1 << BitVector::HexFormat << *bv;
    AssertTrue(std::strcmp(buf, sstr1.str().c_str()) == 0, "BitVector::ToStringHex produces same result as operator<<");

    bv->ToString(buf, sizeof(buf));
    AssertTrue(std::strcmp(buf, expectedBinString) == 0, "BitVector::ToString produces expected result");
    std::stringstream sstr2;
    sstr2 << *bv;
    AssertTrue(std::strcmp(buf, sstr2.str().c_str()) == 0, "BitVector::ToString produces same result as operator<<");

    return true;
}

bool BitVectorSanityTests() {
    BitVector bv;
    bv.SetBitCount(20);
    bv.SetInt(0xff, 0, 8);
    TestBitVectorToString(&bv, "00000000000011111111", "0000ff");
    bv.SetInt(0xff, 18, 2);
    TestBitVectorToString(&bv, "11000000000011111111", "0c00ff");
    bv.SetInt(524288, 0, 20);
    TestBitVectorToString(&bv, "10000000000000000000", "080000");
    bv.SetInt(0xff, 4, 8);
    TestBitVectorToString(&bv, "10000000111111110000", "080ff0");
    bv.Clear();
    bv.Set(16);
    TestBitVectorToString(&bv, "00010000000000000000", "010000");
    
    return true;
}

int main(int argc, const char** argv) {
    const char* target = nullptr;
    if (argc > 1) {
        target = argv[1];
    }
    ReturnErrorIfFalse(TestSolveMatchingProblem(target));

    ReturnErrorIfFalse(TestCrossoverGenes(10, 1));
    ReturnErrorIfFalse(TestCrossoverGenes(10, 7));
    ReturnErrorIfFalse(TestCrossoverGenes(10, 8));
    ReturnErrorIfFalse(TestCrossoverGenes(10, 9));

    ReturnErrorIfFalse(BitVectorSanityTests());

    std::cout << "All tests passed!" << std::endl;

    return 0;
}
