//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#include "BitVector.h"
#include "GeneticAlgorithm.h"
#include "Individual.h"

#define AssertTrue(expr, msg) \
if (!(expr)) { \
    std::cerr << "Failure: " << (msg) << " (" << #expr << ")" << std::endl; \
    assert(expr); \
    return false; \
}

#define ReturnErrorIfFalse(expr) \
if (!(expr)) { \
    std::cerr << "Failure: " << #expr << std::endl; \
    return -1; \
}

using panga::BitVector;
using panga::GeneticAlgorithm;
using panga::Genome;
using panga::Individual;
using panga::RandomWrapper;

namespace testing {

bool VerboseOutput = false;

struct TestUserData {
    size_t current_individual;
    size_t current_generation;
    BitVector target_bits;
};

double TestObjective(Individual* individual, void* user_test_data) {
    auto* test_data = static_cast<TestUserData*>(user_test_data);
    const auto fail_bits = test_data->target_bits.HammingDistance(*individual);

    if (VerboseOutput) {
      std::cout << "Individual " << test_data->current_generation << ":" << test_data->current_individual << " => Error " << fail_bits << std::endl;
    }

    test_data->current_individual++;

    return static_cast<double>(fail_bits);
}

bool TestSolveMatchingProblem(const char* target) {
    GeneticAlgorithm ga;
    Genome& genome = ga.GetGenome();
    TestUserData test_data;

    if (target != nullptr) {
        const size_t len = strlen(target);
        test_data.target_bits.FromString(target, len);
    } else {
        constexpr size_t test_bit_count = 2000U;
        test_data.target_bits.SetBitCount(test_bit_count);
    }

    genome.AddBooleanGenes(test_data.target_bits.GetBitCount());
    AssertTrue(genome.BitsRequired() == test_data.target_bits.GetBitCount(), "Genome encodes correct number of bits");

    constexpr size_t population_size = 100;
    constexpr size_t total_generations = 100;
    constexpr double mutation_rate = 0.0005;
    constexpr double crossover_rate = 0.99;
    constexpr double mutated_elite_rate = 0.5;
    constexpr size_t tournament_size = 5;
    constexpr size_t k_point_count = 5;

    ga.SetPopulationSize(population_size);
    ga.SetTotalGenerations(total_generations);
    ga.SetMutationRate(mutation_rate);
    ga.SetCrossoverRate(crossover_rate);
    ga.SetEliteCount(1);
    ga.SetMutatedEliteCount(0);
    ga.SetMutatedEliteMutationRate(mutated_elite_rate);
    ga.SetFitnessFunction(TestObjective);
    ga.SetMutationRateSchedule(GeneticAlgorithm::MutationRateSchedule::Proportional);
    ga.SetCrossoverType(GeneticAlgorithm::CrossoverType::Uniform);
    ga.SetMutatorType(GeneticAlgorithm::MutatorType::Flip);
    ga.SetSelectorType(GeneticAlgorithm::SelectorType::Tournament);
    ga.SetTournamentSize(tournament_size);
    ga.SetKPointCrossoverPointCount(k_point_count);
    ga.SetProportionalMutationBitCount(1);
    ga.SetAllowSameParentCouples(true);
    ga.SetUserData(&test_data);
    ga.Initialize();

    // Minimum score will fall below 1 when we've solved the problem
    constexpr size_t max_generation = 10000;
    while (ga.GetCurrentGeneration() < max_generation) {
        // Pass the current generation and individual to the fitness function.
        // Reset current individual for this population
        test_data.current_individual = 1;
        // Set this before we call Step (which calls the fitness function) so
        // add one to the current generation.
        test_data.current_generation = ga.GetCurrentGeneration() + 1;

        ga.Step();
        const auto& population = ga.GetPopulation();

        if (VerboseOutput) {
            std::cout << "Generation " << ga.GetCurrentGeneration()
                << " => avg: " << population.GetAverageScore()
                << " min: " << population.GetMinimumScore()
                << " stdev: " << population.GetScoreStandardDeviation()
                << " popdiv: " << population.GetPopulationDiversity()
                << std::endl;
        }

        if (population.GetMinimumScore() < 1.0) { 
            return true;
        }
    }

    return false;
}

bool TestCrossoverGenes(size_t gene_count, size_t gene_width) {
    Genome genome;
    for (size_t i = 0; i < gene_count; i++) {
        const size_t index = genome.AddGene(gene_width);
        AssertTrue(index == i, "Gene index");
    }
    AssertTrue(genome.BitsRequired() == gene_width * gene_count, "Genes add up to the corect length");

    Individual left(genome);
    Individual right(genome);
    Individual offspring(genome);
    AssertTrue(genome.BitsRequired() == left.GetBitCount(), "Individuals based on a genome have length eqaul to the genome bits required");

    // Alternate gene values between all 0s and all 1s
    for (size_t i = 0; i < gene_count; i++) {
        const uint64_t left_value = i % 2U == 0 ? std::numeric_limits<uint64_t>::max() : 0;
        const uint64_t right_value = i % 2U == 0 ? 0 : std::numeric_limits<uint64_t>::max();
        left.EncodeIntegerGene<uint64_t, false>(i, left_value);
        right.EncodeIntegerGene<uint64_t, false>(i, right_value);
    }

    RandomWrapper random;

    // Perform crossover respecting gene boundaries.
    // This means we will take all the bits of a gene instead of cutting them up.
    Individual::UniformCrossover(left, right, &offspring, &random, false);

    if (VerboseOutput) {
        std::cout << "Left: " << left << std::endl << "Right: " << right << std::endl << "Offspring: " << offspring << std::endl;
    }

    for (size_t i = 0; i < gene_count; i++) {
        const auto value = offspring.DecodeIntegerGene<uint64_t, false>(i);
        const auto max_value = std::numeric_limits<uint64_t>::max() >> (sizeof(uint64_t) * CHAR_BIT - gene_width);
        AssertTrue(value == 0 || value == max_value, "Decoded value should be all 1s or all 0s");
    }

    return true;
}

bool TestBitVectorToString(BitVector* bv, const char* expectedBinString, const char* expectedHexString) {
    constexpr size_t buf_size = 2000;
    char buf[buf_size];
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

    BitVector tbv;
    tbv.FromString(expectedBinString, strlen(expectedBinString));
    AssertTrue(bv->Equals(tbv), "BitVector::FromString produces correct BitVector");

    tbv.FromStringHex(expectedHexString, strlen(expectedHexString));
    AssertTrue(bv->Equals(tbv), "BitVector::FromStringHex produces correct BitVector");

    return true;
}

struct BitVectorInt {
    int value;
    size_t bit_index;
    size_t bit_width;
};

struct BitVectorTest {
    size_t bit_count;
    std::vector<BitVectorInt> ints;
    std::vector<size_t> bits;
    const char* bin_string;
    const char* hex_string;
};

const BitVectorTest BitVectorTests[] = {
    {20, {{0xff, 0, 8}}, {}, "00000000000011111111", "0000ff"},
    {20, {{0xff, 0, 8}, {0xff, 18, 2}}, {}, "11000000000011111111", "0c00ff"},
    {20, {{0xff, 0, 8}, {0xff, 18, 2}, {524288, 0, 20}}, {}, "10000000000000000000", "080000"},
    {20, {{0xff, 0, 8}, {0xff, 18, 2}, {524288, 0, 20}, {0xff, 4, 8}}, {}, "10000000111111110000", "080ff0"},
    {20, {}, {16}, "00010000000000000000", "010000"},
    {100, {{0xfff, 1, 16}, {0xfff, 81, 16}}, {}, "0000000111111111111000000000000000000000000000000000000000000000000000000000000000000001111111111110", "001ffe00000000000000001ffe"},

};

bool BitVectorSanityTests() {
    for (const auto& test : BitVectorTests) {
        BitVector bv;
        bv.SetBitCount(test.bit_count);
        for (const auto& i : test.ints) {
            bv.SetInt(i.value, i.bit_index, i.bit_width);
        }
        for (const auto& b : test.bits) {
            bv.Set(b);
        }
        TestBitVectorToString(&bv, test.bin_string, test.hex_string);
    }

    return true;
}

int DoTests(int argc, const char** argv) {
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

}  // namespace testing

int main(int argc, const char** argv) {
  try {
    return testing::DoTests(argc, argv);
  } catch (...) {
    std::cout << "Caught exception running tests.";
    return -1;
  }
  return 0;
}
