//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "Individual.h"
#include "Population.h"
#include "RandomWrapper.h"

namespace panga {

void Population::InitializePartialSums() {
    assert(!empty());

    partial_sums_.resize(size());

    // Assume population is sorted, and fitness values are calculated.
    // First Individual should have the highest fitness score.
    partial_sums_[0] = GetIndividual(0).GetFitness();
    for (size_t i = 1; i < this->size(); i++) {
        partial_sums_[i] = GetIndividual(i).GetFitness() + partial_sums_[i - 1];
    }

    // Scale from (0,1)
    const auto& max_value = partial_sums_[size() - 1U];
    for (auto& i : partial_sums_) {
        i /= max_value;
    }
}

void Population::Sort() {
    // Only initialize the set of sorted indices once or when the size of the population has changed.
    if (sorted_indices_.empty() || sorted_indices_.size() != size()) {
        sorted_indices_.resize(size());
        for (size_t index = 0; index < size(); index++) {
            sorted_indices_[index] = index;
        }
    }

    assert(sorted_indices_.size() == size());
    std::sort(sorted_indices_.begin(), sorted_indices_.end(), [=](const size_t& left, const size_t& right) { return this->at(left) < this->at(right); });

    is_sorted_ = true;
}

const Individual& Population::GetBestIndividual() const {
    // Assume we are sorted and best individual is stored at 0th index.
    return GetIndividual(0);
}

const Individual& Population::GetIndividual(size_t index) const {
    assert(!sorted_indices_.empty());
    assert(!empty());
    assert(index < sorted_indices_.size());
    assert(is_sorted_);

    const auto sorted_index = sorted_indices_[index];
    return this->at(sorted_index);
}

double Population::GetMinimumScore() const {
    return GetBestIndividual().GetScore();
}

double Population::GetAverageScore() const {
    assert(!empty());

    double sum = 0.0;
    for (const auto& i : *this) {
        sum += i.GetScore();
    }
    return sum / size();
}

double Population::GetScoreStandardDeviation() const {
    // stdev of a single score is 0.
    if (size() <= 1U) {
        return 0.0;
    }

    const double mean = GetAverageScore();
    double sum = 0.0;
    for (const auto& i : *this) {
        const double score = i.GetScore();
        sum += (score - mean) * (score - mean);
    }
    return std::sqrt(sum / (size() - 1U));
}

double Population::GetPopulationDiversity() const {
    // Diversity of a single individual would be zero.
    if (size() <= 1U) {
        return 0.0;
    }

    size_t distance = 0;
    size_t j = 0;

    // Compute the total number of differing bits between each individual and every other in the population.
    for (const auto& i : *this) {
        // Compare the individual only against members at further indices in the population. This way we don't double count each comparison.
        for (auto it = cbegin() + ++j; it != cend(); it++) {
            distance += i.HammingDistance(*it);
        }
    }

    const size_t genome_bits = at(0).GetGenome().BitsRequired();
    const size_t total_compares = (size() * (size() - 1U)) >> 2U;
    const size_t total_bits = total_compares * genome_bits;
    return static_cast<double>(distance) / total_bits;
}

const Individual& Population::UniformSelect(RandomWrapper* random) const {
    assert(!empty());

    size_t index = random->RandomInteger<size_t>(0, size() - 1U);
    return at(index);
}

const Individual& Population::RouletteWheelSelect(RandomWrapper* random) const {
    assert(!empty());
    assert(size() == partial_sums_.size());

    const double cutoff = random->RandomFloat<double>(0.0, 1.0);
    size_t lower = 0;
    size_t upper = size() - 1U;

    // Perform binary search across partial sums.
    while (upper >= lower) {
        const auto i = lower + (upper - lower) / 2U;
        if (partial_sums_[i] > cutoff) {
            upper = i - 1;
        } else {
            lower = i + 1;
        }
    }

    lower = std::min(size() - 1U, lower);
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
