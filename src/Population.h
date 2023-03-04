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

    /**
     * Sort the individuals in the population by decreasing fitness - ie: the best individual will be stored at index 0, the second best at index 1, etc.
     */
    void Sort();

    /**
     * Return the Individual with highest fitness value in the population.<br/>
     * Note: Requires the population to have been sorted.
     */
    const Individual& GetBestIndividual() const;

    /**
     * Return the Individual at |index| position in the population.<br/>
     * Note: Requires the population to have been sorted.
     */
    const Individual& GetIndividual(size_t index) const;

    /**
     * Get the minimum score among individuals in the population.
     */
    double GetMinimumScore() const;

    /**
     * Get the average score between individuals in the population.
     */
    double GetAverageScore() const;

    /**
     * Get the standard deviation of scores between individuals in the population.
     */
    double GetScoreStandardDeviation() const;

    /**
     * Get the population diversity among individuals in the population.<br/>
     * The population diversity is a metric used to indicate how much the
     * genetic material backing the individuals varies. A high diversity value
     * means the individuals have very different genetic components. A value
     * of zero means the individuals are identical.
     */
    double GetPopulationDiversity() const;

    /**
     * Select an individual from the population at random.
     */
    const Individual& UniformSelect(RandomWrapper* random) const;
    const Individual& RouletteWheelSelect(RandomWrapper* random) const;
    const Individual& TournamentSelect(size_t tournament_size, RandomWrapper* random) const;

    /**
     * Select the individual in the population with highest fitness score.
     */
    const Individual& RankSelect() const;

private:
    std::vector<double> partial_sums_;
    std::vector<size_t> sorted_indices_;
};

}  // namespace panga

#endif  // POPULATION_H__
