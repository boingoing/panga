//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef POPULATION_H__
#define POPULATION_H__

#include <vector>

namespace panga {

class BitVector;
class Genome;
class Individual;
class RandomWrapper;

using FitnessFunction = double(*)(Individual*, void*);

/**
 * Just a collection of Individual objects.<br/>
 * We can select Individuals from this population according to their
 * fitness values.<br/>
 * Note: In order for the select functions to work correctly, the population
 * must be sorted in decreasing fitness order with the Individual at index 0
 * being the most fit.
 */
class Population {
public:
    Population() = delete;
    explicit Population(const Genome& genome);
    Population(const Population& rhs) = delete;
    Population(Population&& rhs) = default;
    Population& operator=(const Population& rhs) = delete;
    ~Population() = default;

    /**
     * Return the number of individuals in the population.
     */
    size_t Size() const;

    /**
     * Resize the population to hold |size| individuals.<br/>
     * If |size| is greater than the current size of the population, new random individuals will be added until we reach |size| individuals.
     */
    void Resize(size_t size, RandomWrapper* random);

    /**
     * Initialize the population based on |initial_population|.<br/>
     * Clears any individuals currently in the population.
     * @param initial_population We will construct new population members and interpret these values as binary chromosome data.
     */
    void Initialize(const std::vector<BitVector>& initial_population);

    /**
     * Initialize the set of partial sums we use for the roulette wheel selector.<br/>
     * Note: Requires the population to have been sorted.
     */
    void InitializePartialSums();

    /**
     * Sort the individuals in the population by decreasing fitness - ie: the best individual will be stored at index 0, the second best at index 1, etc.<br/>
     * This must be called before using the selection functions as they rely on the population being sorted.
     */
    void Sort();

    /**
     * Replace the individual currently at |index| in the population with |individual|.
     */
    void Replace(size_t index, const Individual& individual);

    /**
     * Return the Individual with highest fitness value in the population.<br/>
     * Note: Requires the population to have been sorted.
     */
    const Individual& GetBestIndividual() const;

    /**
     * Return the Individual at |index| position in the population based on fitness where the individual at index 0 is the most fit, the second most fit is at index 1, etc.<br/>
     * Note: Requires the population to have been sorted.
     */
    const Individual& GetIndividual(size_t index) const;

    /**
     * Get a writable reference to the individual at |index| position in the population.<br/>
     * Does not require the population to be sorted and does not return individuals in relative order of fitness.<br/>
     * Note: Modifying individuals in the population while the genetic algorithm is running may lead to undexpected results.
     */
    Individual& GetIndividualWritable(size_t index);

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

    /**
     * Spin a roulette wheel to select an individual where each slice on the wheel corresponds to the partial sum of a ranked individual. The size of each partial sum on the wheel is relative to the fitness of that individual with more fit individuals having a larger-sized slice.<br/>
     * Patial sums must have already been created via InitializePartialSums.
     * @see InitializePartialSums
     */
    const Individual& RouletteWheelSelect(RandomWrapper* random) const;

    /**
     * Randomly select |tournament_size| individuals from the population and return the one with highest fitness.
     */
    const Individual& TournamentSelect(size_t tournament_size, RandomWrapper* random) const;

    /**
     * Select the individual in the population with highest fitness score.
     */
    const Individual& RankSelect() const;

    void Evaluate(FitnessFunction fitness_function, void* user_data);

private:
    const Genome& genome_;
    std::vector<Individual> individuals_;
    std::vector<double> partial_sums_;
    std::vector<size_t> sorted_indices_;
    bool is_sorted_ = false;
};

}  // namespace panga

#endif  // POPULATION_H__
