//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>

namespace panga {

class Individual;
class RandomWrapper;

/**
 * Just a collection of Individual objects.<br/>
 * We can select Individuals from this population according to their
 * fitness values.<br/>
 * Note: In order for the select functions to work correctly, the population
 * must be sorted in decreasing fitness order with the Individual at index 0
 * being the most fit.
 */
class Population : public std::vector<Individual*> {
protected:
    std::vector<double> _partialSums;

public:
    void InitializePartialSums();

    Individual* UniformSelect(RandomWrapper* randomWrapper) const;
    Individual* RouletteWheelSelect(RandomWrapper* randomWrapper) const;
    Individual* TournamentSelect(size_t tournamentSize, RandomWrapper* randomWrapper) const;
    Individual* RankSelect() const;
};

} // namespace panga
