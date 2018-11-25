//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "Population.h"
#include "Individual.h"
#include "RandomWrapper.h"

using namespace panga;

void Population::InitializePartialSums() {
    assert(this->size() > 0);

    this->_partialSums.resize(this->size());

    // Assume population is sorted, and fitness values are calculated.
    // First Individual should have the highest fitness score.
    this->_partialSums[0] = this->at(0)->GetFitness();

    for (size_t i = 1; i < this->size(); i++) {
        this->_partialSums[i] = this->at(i)->GetFitness() + this->_partialSums[i - 1];
    }

    // Scale from (0,1)
    for (size_t i = 0; i < this->size(); i++) {
        this->_partialSums[i] /= this->_partialSums[this->size() - 1];
    }
}

Individual* Population::UniformSelect(RandomWrapper* randomWrapper) const {
    assert(this->size() > 0);

    size_t index = randomWrapper->RandomInteger<size_t>(0, this->size() - 1);
    return this->at(index);
}


Individual* Population::RouletteWheelSelect(RandomWrapper* randomWrapper) const {
    assert(this->size() > 0);
    assert(this->size() == this->_partialSums.size());

    double cutoff = randomWrapper->RandomFloat<double>(0.0, 1.0);
    int lower = 0;
    int upper = (int)this->size() - 1;

    // Perform binary search across partial sums.
    while (upper >= lower) {
        int i = lower + (upper - lower) / 2;

        if (this->_partialSums[i] > cutoff) {
            upper = i - 1;
        } else {
            lower = i + 1;
        }
    }

    lower = std::min((int)this->size() - 1, lower);
    return this->at(lower);
}

Individual* Population::TournamentSelect(size_t tournamentSize, RandomWrapper* randomWrapper) const {
    Individual* selected = nullptr;

    // Choose random individuals from the population to be part of the tournament.
    for (size_t i = 0; i < tournamentSize; i++) {
        Individual* temp = this->UniformSelect(randomWrapper);

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
