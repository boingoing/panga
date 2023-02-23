//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef POPULATION_H__
#define POPULATION_H__

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
class Population : public std::vector<Individual> {
public:
    Population() = default;
    Population(const Population& rhs) = default;
    Population& operator=(const Population& rhs) = default;
    ~Population() = default;

    void InitializePartialSums();

    const Individual& UniformSelect(RandomWrapper* random) const;
    const Individual& RouletteWheelSelect(RandomWrapper* random) const;
    const Individual& TournamentSelect(size_t tournament_size, RandomWrapper* random) const;
    const Individual& RankSelect() const;

private:
    std::vector<double> partial_sums_;
};

}  // namespace panga

#endif  // POPULATION_H__
