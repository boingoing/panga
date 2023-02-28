//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "Individual.h"
#include "Population.h"
#include "RandomWrapper.h"

namespace panga {

void Population::InitializePartialSums() {
    assert(size() > 0);

    this->partial_sums_.resize(this->size());

    // Assume population is sorted, and fitness values are calculated.
    // First Individual should have the highest fitness score.
    this->partial_sums_[0] = this->at(0)->GetFitness();

    for (size_t i = 1; i < this->size(); i++) {
        this->partial_sums_[i] = this->at(i)->GetFitness() + this->partial_sums_[i - 1];
    }

    // Scale from (0,1)
    for (size_t i = 0; i < this->size(); i++) {
        this->partial_sums_[i] /= this->partial_sums_[this->size() - 1];
    }
}

void Population::Sort() {
    sorted_indices_.resize(size());

    for (size_t index = 0; index < size(); index++) {
        
    }
}

const Individual& Population::GetBestIndividual() const {

}

const Individual& Population::UniformSelect(RandomWrapper* random) const {
    assert(size() > 0);

    size_t index = random->RandomInteger<size_t>(0, size() - 1U);
    return at(index);
}

Individual* Population::RouletteWheelSelect(RandomWrapper* random) const {
    assert(this->size() > 0);
    assert(this->size() == this->partial_sums_.size());

    double cutoff = random->RandomFloat<double>(0.0, 1.0);
    int lower = 0;
    int upper = (int)this->size() - 1;

    // Perform binary search across partial sums.
    while (upper >= lower) {
        int i = lower + (upper - lower) / 2;

        if (this->partial_sums_[i] > cutoff) {
            upper = i - 1;
        } else {
            lower = i + 1;
        }
    }

    lower = std::min((int)this->size() - 1, lower);
    return this->at(lower);
}

Individual* Population::TournamentSelect(size_t tournament_size, RandomWrapper* random) const {
    Individual* selected = nullptr;

    // Choose random individuals from the population to be part of the tournament.
    for (size_t i = 0; i < tournament_size; i++) {
        Individual* temp = this->UniformSelect(random);

        // If this is the first individual we've picked, it will be the winner for now.
        // Otherwise, choose the most fit between the previous winner and the new 
        // member of the tournament.
        if (selected == nullptr || temp->GetFitness() > selected->GetFitness()) {
            selected = temp;
        }
    }

    return selected;
}

Individual* Population::RankSelect() const {
    assert(this->size() > 0);

    // Population is assumed to be sorted with the best individual in the 0 position.
    return this->at(0);
}

}  // namespace panga
