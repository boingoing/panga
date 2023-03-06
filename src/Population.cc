//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "Individual.h"
#include "Population.h"
#include "RandomWrapper.h"

namespace panga {

Population::Population(const Genome& genome) : genome_(genome) {}

size_t Population::Size() const {
    return individuals_.size();
}

void Population::Resize(size_t size, RandomWrapper* random) {
    // If we're shrinking the population, no need to construct new individuals.
    if (individuals_.size() >= size) {
        individuals_.resize(size);
        return;
    }

    // If we're inserting new random individuals, construct them in the vector.
    while (individuals_.size() < size) {
        auto& individual = individuals_.emplace_back(genome_);
        individual.Randomize(random);
    }
}

void Population::Initialize(const std::vector<BitVector>& initial_population) {
    // Destroy any individuals currently in the population and construct new ones based on the |initial_population| BVs.
    individuals_.clear();

    for (const auto& bv : initial_population) {
        individuals_.emplace_back(genome_, bv);
    }
}

void Population::InitializePartialSums() {
    assert(!individuals_.empty());

    partial_sums_.resize(individuals_.size());

    // Assume population is sorted, and fitness values are calculated.
    // First Individual should have the highest fitness score.
    partial_sums_[0] = GetIndividual(0).GetFitness();
    for (size_t i = 1; i < individuals_.size(); i++) {
        partial_sums_[i] = GetIndividual(i).GetFitness() + partial_sums_[i - 1];
    }

    // Scale from (0,1)
    const auto& max_value = partial_sums_[individuals_.size() - 1U];
    for (auto& i : partial_sums_) {
        i /= max_value;
    }
}

void Population::Sort() {
    // Only initialize the set of sorted indices once or when the size of the population has changed.
    if (sorted_indices_.empty() || sorted_indices_.size() != individuals_.size()) {
        sorted_indices_.resize(individuals_.size());
        for (size_t index = 0; index < individuals_.size(); index++) {
            sorted_indices_[index] = index;
        }
    }

    assert(sorted_indices_.size() == individuals_.size());
    std::sort(sorted_indices_.begin(), sorted_indices_.end(), [=](const size_t& left, const size_t& right) { return individuals_[left] < individuals_[right]; });

    is_sorted_ = true;
}

void Population::Replace(size_t index, const Individual& individual) {
    assert(individuals_.size() > index);

    // TODO(boingoing): Should we replace based on sorted index?
    individuals_[index] = individual;
}

const Individual& Population::GetBestIndividual() const {
    // Assume we are sorted and best individual is stored at 0th index.
    return GetIndividual(0);
}

const Individual& Population::GetIndividual(size_t index) const {
    assert(!sorted_indices_.empty());
    assert(!individuals_.empty());
    assert(index < sorted_indices_.size());
    assert(is_sorted_);

    const auto sorted_index = sorted_indices_[index];
    return individuals_[sorted_index];
}

Individual& Population::GetIndividualWritable(size_t index) {
    assert(index < individuals_.size());
    return individuals_[index];
}

double Population::GetMinimumScore() const {
    return GetBestIndividual().GetScore();
}

double Population::GetAverageScore() const {
    assert(!individuals_.empty());

    double sum = 0.0;
    for (const auto& i : individuals_) {
        sum += i.GetScore();
    }
    return sum / individuals_.size();
}

double Population::GetScoreStandardDeviation() const {
    // stdev of a single score is 0.
    if (individuals_.size() <= 1U) {
        return 0.0;
    }

    const double mean = GetAverageScore();
    double sum = 0.0;
    for (const auto& i : individuals_) {
        const double score = i.GetScore();
        sum += (score - mean) * (score - mean);
    }
    return std::sqrt(sum / (individuals_.size() - 1U));
}

double Population::GetPopulationDiversity() const {
    // Diversity of a single individual would be zero.
    if (individuals_.size() <= 1U) {
        return 0.0;
    }

    size_t distance = 0;
    size_t j = 0;

    // Compute the total number of differing bits between each individual and every other in the population.
    for (const auto& i : individuals_) {
        // Compare the individual only against members at further indices in the population. This way we don't double count each comparison.
        for (auto it = individuals_.cbegin() + ++j; it != individuals_.cend(); it++) {
            distance += i.HammingDistance(*it);
        }
    }

    const size_t genome_bits = individuals_[0].GetGenome().BitsRequired();
    const size_t total_compares = (individuals_.size() * (individuals_.size() - 1U)) >> 2U;
    const size_t total_bits = total_compares * genome_bits;
    return static_cast<double>(distance) / total_bits;
}

void Population::Evaluate(FitnessFunction fitness_function, void* user_data) {
    // Score members of population.
    for (auto& individual : individuals_) {
        individual.SetScore(fitness_function(&individual, user_data));
    }

    // Sort the population by increasing raw score.
    Sort();

    // Calculate the Individual fitness scores.
    double fitness_sum = 0.0;
    const double best_score = GetBestIndividual().GetScore();
    const double worst_score = GetIndividual(Size() - 1U).GetScore();

    // We need to invert the trend of scores.
    // First, calculate best+worst - score[i] as an intermediate fitness score.
    for (auto& individual : individuals_) {
        const double temp_fitness = best_score + worst_score - individual.GetScore();
        fitness_sum += temp_fitness;
        individual.SetFitness(temp_fitness);
    }

    // Now calculate fitness as a proportion of the intermediate sum.
    for (auto& individual : individuals_) {
        individual.SetFitness(individual.GetFitness() / fitness_sum);
    }
}

const Individual& Population::UniformSelect(RandomWrapper* random) const {
    assert(!individuals_.empty());

    size_t index = random->RandomInteger<size_t>(0, individuals_.size() - 1U);
    return individuals_[index];
}

const Individual& Population::RouletteWheelSelect(RandomWrapper* random) const {
    assert(!individuals_.empty());
    assert(individuals_.size() == partial_sums_.size());

    const double cutoff = random->RandomFloat<double>(0.0, 1.0);
    size_t lower = 0;
    size_t upper = individuals_.size() - 1U;

    // Perform binary search across partial sums.
    while (upper >= lower) {
        const auto i = lower + (upper - lower) / 2U;
        if (partial_sums_[i] > cutoff) {
            upper = i - 1;
        } else {
            lower = i + 1;
        }
    }

    lower = std::min(individuals_.size() - 1U, lower);
    return GetIndividual(lower);
}

const Individual& Population::TournamentSelect(size_t tournament_size, RandomWrapper* random) const {
    assert(tournament_size > 0);

    const Individual* selected = nullptr;

    // Choose random individuals from the population to be part of the tournament.
    for (size_t i = 0; i < tournament_size; i++) {
        const auto& temp = UniformSelect(random);

        // If this is the first individual we've picked, it will be the winner for now.
        // Otherwise, choose the most fit between the previous winner and the new 
        // member of the tournament.
        if (selected == nullptr || temp.GetFitness() > selected->GetFitness()) {
            selected = &temp;
        }
    }

    assert(selected != nullptr);
    return *selected;
}

const Individual& Population::RankSelect() const {
    return GetBestIndividual();
}

}  // namespace panga
