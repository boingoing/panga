//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <algorithm>
#include <numeric>
#include <utility>

#include "GeneticAlgorithm.h"
#include "Individual.h"

namespace panga {

GeneticAlgorithm::GeneticAlgorithm() : population_(genome_), last_generation_population_(genome_) {
    populations_.emplace_back(genome_);
    populations_.emplace_back(genome_);
}

Genome& GeneticAlgorithm::GetGenome() {
    return genome_;
}

void GeneticAlgorithm::SetMutatedEliteMutationRate(double mutated_elite_mutation_rate) {
    mutated_elite_mutation_rate_ = mutated_elite_mutation_rate;
}

double GeneticAlgorithm::GetMutatedEliteMutationRate() const {
    return mutated_elite_mutation_rate_;
}

void GeneticAlgorithm::SetMutationRate(double mutation_rate) {
    mutation_rate_ = mutation_rate;
}

double GeneticAlgorithm::GetMutationRate() const {
    return mutation_rate_;
}

void GeneticAlgorithm::SetCrossoverRate(double crossover_rate) {
    crossover_rate_ = crossover_rate;
}

double GeneticAlgorithm::GetCrossoverRate() const {
    return crossover_rate_;
}

void GeneticAlgorithm::SetPopulationSize(size_t population_size) {
    population_size_ = population_size;
}

size_t GeneticAlgorithm::GetPopulationSize() const {
    return population_size_;
}

void GeneticAlgorithm::SetTotalGenerations(size_t total_generations) {
    total_generations_ = total_generations;
}

size_t GeneticAlgorithm::GetTotalGenerations() const {
    return total_generations_;
}

void GeneticAlgorithm::SetFitnessFunction(FitnessFunction fitness_function) {
    fitness_function_ = fitness_function;
}

GeneticAlgorithm::FitnessFunction GeneticAlgorithm::GetFitnessFunction() const {
    return fitness_function_;
}

size_t GeneticAlgorithm::GetEliteCount() const {
    return elite_count_;
}

void GeneticAlgorithm::SetEliteCount(size_t elite_count) {
    elite_count_ = elite_count;
}

size_t GeneticAlgorithm::GetMutatedEliteCount() const {
    return mutated_elite_count_;
}

void GeneticAlgorithm::SetMutatedEliteCount(size_t mutated_elite_count) {
    mutated_elite_count_ = mutated_elite_count;
}

void GeneticAlgorithm::SetMutationRateSchedule(MutationRateSchedule mutation_rate_schedule) {
    mutation_rate_schedule_ = mutation_rate_schedule;
}

GeneticAlgorithm::MutationRateSchedule GeneticAlgorithm::GetMutationRateSchedule() const {
    return mutation_rate_schedule_;
}

void GeneticAlgorithm::SetUserData(void* user_data) {
    user_data_ = user_data;
}

void* GeneticAlgorithm::GetUserData() const {
    return user_data_;
}

void GeneticAlgorithm::SetCrossoverType(CrossoverType crossover_type) {
    crossover_type_ = crossover_type;
}

GeneticAlgorithm::CrossoverType GeneticAlgorithm::GetCrossoverType() const {
    return crossover_type_;
}

void GeneticAlgorithm::SetMutatorType(MutatorType mutator_type) {
    mutator_type_ = mutator_type;
}

GeneticAlgorithm::MutatorType GeneticAlgorithm::GetMutatorType() const {
    return mutator_type_;
}

void GeneticAlgorithm::SetSelectorType(SelectorType selector_type) {
    selector_type_ = selector_type;
}

GeneticAlgorithm::SelectorType GeneticAlgorithm::GetSelectorType() const {
    return selector_type_;
}

void GeneticAlgorithm::SetCrossoverIgnoreGeneBoundaries(bool crossover_ignore_gene_boundaries) {
    crossover_ignore_gene_boundaries_ = crossover_ignore_gene_boundaries;
}

bool GeneticAlgorithm::GetCrossoverIgnoreGeneBoundaries() const {
    return crossover_ignore_gene_boundaries_;
}

void GeneticAlgorithm::SetAllowSameParentCouples(bool allow_same_parent_couples) {
    allow_same_parent_couples_ = allow_same_parent_couples;
}

bool GeneticAlgorithm::GetAllowSameParentCouples() const {
    return allow_same_parent_couples_;
}

size_t GeneticAlgorithm::GetCurrentGeneration() const {
    return current_generation_;
}

void GeneticAlgorithm::SetTournamentSize(size_t tournament_size) {
    tournament_size_ = tournament_size;
}

size_t GeneticAlgorithm::GetTournamentSize() const {
    return tournament_size_;
}

void GeneticAlgorithm::SetKPointCrossoverPointCount(size_t k_point_crossover_point_count) {
    k_point_crossover_point_count_ = k_point_crossover_point_count;
}

size_t GeneticAlgorithm::GetKPointCrossoverPointCount() const {
    return k_point_crossover_point_count_;
}

void GeneticAlgorithm::SetSelfAdaptiveMutationDiversityFloor(double self_adaptive_mutation_diversity_floor) {
    self_adaptive_mutation_diversity_floor_ = self_adaptive_mutation_diversity_floor;
}

double GeneticAlgorithm::GetSelfAdaptiveMutationDiversityFloor() const {
    return self_adaptive_mutation_diversity_floor_;
}

void GeneticAlgorithm::SetSelfAdaptiveMutationAggressiveRate(double self_adaptive_mutation_aggressive_rate) {
    self_adaptive_mutation_aggressive_rate_ = self_adaptive_mutation_aggressive_rate;
}

double GeneticAlgorithm::GetSelfAdaptiveMutationAggressiveRate() const {
    return self_adaptive_mutation_aggressive_rate_;
}

void GeneticAlgorithm::SetProportionalMutationBitCount(size_t proportional_mutation_bit_count) {
    proportional_mutation_bit_count_ = proportional_mutation_bit_count;
}

size_t GeneticAlgorithm::GetProportionalMutationBitCount() const {
    return proportional_mutation_bit_count_;
}

const Population& GeneticAlgorithm::GetPopulation() const {
    return last_generation_population_;
}

void GeneticAlgorithm::SetInitialPopulation(const std::vector<const BitVector>& initial_population) {
    auto& population = GetCurrentPopulation();
    population.Initialize(initial_population);
}

void GeneticAlgorithm::Initialize() {
    // If the population isn't full, this will fill it up with random individuals.
    auto& population = GetCurrentPopulation();
    population.Resize(population_size_, &random_);

    // Reset the current generation.
    current_generation_ = 0;
}

Population& GeneticAlgorithm::GetCurrentPopulation() {
    // Even generations should use the 0th population as current.
    // Odd generations should use the 1st population as current.
    const size_t current_population_index = current_generation_ % 2U;
    return populations_[current_population_index];
}

Population& GeneticAlgorithm::GetLastGenerationPopulation() {
    // Even generations should use the 1st population as previous.
    // Odd generations should use the 0th population as previous.
    const size_t previous_population_index = (current_generation_ + 1U) % 2U;
    return populations_[previous_population_index];
}

void GeneticAlgorithm::Step() {
    auto& current_population = GetCurrentPopulation();

    // If we're on any generation other than the 0th one, we need to build the current population based on the previous generation.
    if (current_generation_ != 0) {
        const auto& last_generation_population = GetLastGenerationPopulation();
        // Elitism
        // Add the best individuals from last generation into the current population.
        // Note: last_generation_population_ must already be sorted with best individuals at the front.
        for (size_t i = 0; i < elite_count_; i++) {
            // Overwrite the first i members of population with those from last generation.
            population_[i] = last_generation_population_[i];
        }

        // Mutated elitism
        // Take the best individuals from the last generation but mutate them by a variable rate.
        for (size_t i = 0; i < mutated_elite_count_; i++) {
            auto& mutated_elite = population_[elite_count_ + i];
            mutated_elite = last_generation_population_[i];
            Mutate(&mutated_elite, mutated_elite_mutation_rate_);
        }

        // Get the mutation rate for the current generation.
        // Note: This can depend on the population already having been evaluated.
        const double current_mutation_rate = GetCurrentMutationRate();
        // Initialize the selector.
        InitializeSelector(&last_generation_population_);

        // We already added elite_count_ + mutated_elite_count_ individuals based on the last generation.
        size_t current_population_size = elite_count_ + mutated_elite_count_;
        // Create offspring from individuals in last generation.
        while (current_population_size < population_size_) {
            auto& offspring = population_[current_population_size++];

            // Select a couple from the last generation.
            const auto parents = SelectParents(last_generation_population_);

            // See if we will do crossover or duplicate a parent.
            if (random_.CoinFlip(crossover_rate_)) {
                Crossover(parents.first, parents.second, &offspring);
            } else {
                // TODO(boingoing): Should we flip an even coin here to decide which parent to duplicate?
                offspring = parents.first;
            }

            // Mutate offspring.
            Mutate(&offspring, current_mutation_rate);
        }
    }

    // Score and sort the current population.
    // This population is either the result of Initialize() or a previous Step() operation.
    Evaluate(&current_population);

    current_generation_++;
}

void GeneticAlgorithm::Evaluate(Population* population) {
    // Score members of population.
    for (auto& individual : *population) {
        individual.SetScore(fitness_function_(&individual, user_data_));
    }

    // Sort the population by increasing raw score.
    std::sort(population->begin(), population->end());

    // Calculate the Individual fitness scores.
    double fitness_sum = 0.0;
    const double best_score = population->front().GetScore();
    const double worst_score = population->back().GetScore();

    // We need to invert the trend of scores.
    // First, calculate best+worst - score[i] as an intermediate fitness score.
    for (auto& individual : *population) {
        const double temp_fitness = best_score + worst_score - individual.GetScore();
        fitness_sum += temp_fitness;
        individual.SetFitness(temp_fitness);
    }

    // Now calculate fitness as a proportion of the intermediate sum.
    for (auto& individual : *population) {
        individual.SetFitness(individual.GetFitness() / fitness_sum);
    }
}

void GeneticAlgorithm::Run() {
    while (current_generation_ < total_generations_) {
        Step();
    }
}

double GeneticAlgorithm::GetCurrentMutationRate() {
    switch (mutation_rate_schedule_) {
    case MutationRateSchedule::Constant:
        return mutation_rate_;
    case MutationRateSchedule::Deterministic:
        // If total generations is 0 or if we are on a generation after the total counter,
        // deterministic calculation would return 0 for mutation rate.
        // In that case just use the constant-schedule mutation rate instead.
        return total_generations_ == 0 || current_generation_ > total_generations_ ?
            mutation_rate_ :
            1.0 / (2.0 + ((genome_.BitsRequired() - 2.0) / (total_generations_ - 1.0)) * current_generation_);
    case MutationRateSchedule::SelfAdaptive:
        if (GetPopulationDiversity() < self_adaptive_mutation_diversity_floor_) {
            return self_adaptive_mutation_aggressive_rate_;
        } else {
            return mutation_rate_;
        }
    case MutationRateSchedule::Proportional:
        // Proportional schedule sets a mutation probability in order to flip proportional_mutation_bit_count_ bits.
        assert(genome_.BitsRequired() != 0);
        return proportional_mutation_bit_count_ * (1.0 / genome_.BitsRequired());
    default:
        // Invalid mutation rate schedule
        assert(false);
        return 0.0;
    }
}

void GeneticAlgorithm::Crossover(const Individual& parent1, const Individual& parent2, Individual* offspring) {
    switch (crossover_type_) {
    case CrossoverType::OnePoint:
        Chromosome::KPointCrossover(1, parent1, parent2, offspring, &random_, crossover_ignore_gene_boundaries_);
        break;
    case CrossoverType::TwoPoint:
        Chromosome::KPointCrossover(2, parent1, parent2, offspring, &random_, crossover_ignore_gene_boundaries_);
        break;
    case CrossoverType::KPoint:
        Chromosome::KPointCrossover(k_point_crossover_point_count_, parent1, parent2, offspring, &random_, crossover_ignore_gene_boundaries_);
        break;
    case CrossoverType::Uniform:
        Chromosome::UniformCrossover(parent1, parent2, offspring, &random_, crossover_ignore_gene_boundaries_);
        break;
    default:
        assert(false);
    }
}

void GeneticAlgorithm::Mutate(Individual* individual, double mutation_percentage) {
    switch (mutator_type_) {
    case MutatorType::Flip:
        Chromosome::FlipMutator(individual, mutation_percentage, &random_);
        break;
    default:
        assert(false);
        break;
    }
}

void GeneticAlgorithm::InitializeSelector(Population* population) {
    if (selector_type_ == SelectorType::RouletteWheel) {
        population->InitializePartialSums();
    }
}

const Individual& GeneticAlgorithm::SelectOne(const Population& population) {
    switch (selector_type_) {
    case SelectorType::Rank:
        return population.RankSelect();
    case SelectorType::Uniform:
        return population.UniformSelect(&random_);
    case SelectorType::RouletteWheel:
        return population.RouletteWheelSelect(&random_);
    case SelectorType::Tournament:
        return population.TournamentSelect(tournament_size_, &random_);
    default:
        assert(false);
    }
}

std::pair<const Individual&, const Individual&> GeneticAlgorithm::SelectParents(const Population& population) {
    assert(!population.empty());

    const auto& first = SelectOne(population);

    // If we can select the same parent for each pair element, we can just select another one and return them.
    if (allow_same_parent_couples_) {
        const auto& second = SelectOne(population);
        return {first, second};
    }

    // When we want to avoid selecting the same parent for each pair element, do the simple thing and select from a population which doesn't contain |first|.
    Population temp;
    for (const auto& i : population) {
        if (&i != &first) {
            temp.push_back(i);
        }
    }
    InitializeSelector(&temp);
    const auto& second = SelectOne(temp);
    return {first, second};
}

}  // namespace panga
