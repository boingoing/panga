//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>

#include "RandomWrapper.h"
#include "Population.h"

namespace panga {

class Genome;
class Individual;
class BitVector;

class GeneticAlgorithm {
public:
    /**
     * Type of crossover we will perform on selected parents in order to produce
     * offspring for the next generation.<br/>
     * Actual crossover is performed by the Chromosome itself.
     * @see Crossover
     * @see SetCrossoverType
     * @see GetCrossoverType
     * @see Chromosome
     */
    enum class CrossoverType : uint8_t {
        /**
         * Perform one-point crossover.<br/>
         * The parent chromosomes will be split into two pieces (based on one cut
         * point) with the offspring receiving the left part of one parent and the
         * right part of the other parent.<br/>
         * The point at which to cut the parent chromosomes is determined randomly.
         * @see Chromosome::KPointCrossover
         */
        OnePoint = 1,

        /**
         * Perform two-point crossover.<br/>
         * Similar to one-point crossover, but the parent chromosomes will be split
         * into three pieces (based on two cut points) with the offspring receiving
         * the first and last part from one parent and the middle part from the
         * other parent.<br/>
         * The points at which the parent chromosomes is cut will be determined
         * randomly.
         * @see Chromosome::KPointCrossover
         */
        TwoPoint,

        /**
         * Perform k-point crossover.<br/>
         * Similar to two-point crossover, but the parent chromosomes will be split
         * into k+1 pieces (based on k cut points) with the offspring receiving
         * pieces from both parents alternating.<br/>
         * The points at which the parent chromosomes is cut will be determined
         * randomly.<br/>
         * k is determined by _kPointCrossoverPointCount.
         * @see _kPointCrossoverPointCount
         * @see SetKPointCrossoverPointCount
         * @see GetKPointCrossoverPointCount
         * @see Chromosome::KPointCrossover
         */
        KPoint,

        /**
         * Perform uniform crossover.<br/>
         * Each bit in the chromosome has an equal chance of being copied from
         * either parent.
         * @see Chromosome::UniformCrossover
         */
        Uniform
    };

    /**
     * The type of mutation we will perform on new individuals when they are added
     * to the next generation population.
     * @see Mutate
     * @see SetMutatorType
     * @see GetMutatorType
     */
    enum class MutatorType : uint8_t {
        /**
         * Perform the flip mutator.<br/>
         * This randomly flips bits in the Individual based on the mutation rate
         * provided.<br/>
         * Every bit in the chromosome has a mutation rate chance of flipping.
         * @see Chromosome::FlipMutator
         */
        Flip = 1
    };

    /**
     * Algorithm we want to use when we select individuals from the current
     * population to become parents of an offspring in the next generation
     * population.<br/>
     * Actual selection is done by the Population itself.
     * @see SetSelectorType
     * @see GetSelectorType
     * @see SelectParents
     * @see SelectOne
     * @see Population
     */
    enum class SelectorType : uint8_t {
        /**
         * Rank selector.<br/>
         * Always returns the fittest individual from a population.<br/>
         * When _allowSameParentCouples is false, returns the top two individuals
         * when we select a couple from a population.
         * @see Population::RankSelect
         */
        Rank = 1,

        /**
         * Uniform selector.<br/>
         * Always returns a random individual from a population.
         * @see Population::UniformSelect
         */
        Uniform,

        /**
         * Roulette wheel selector.<br/>
         * Selects an individual with higher fitness more often but has a chance
         * to select any individual in a population.<br/>
         * This can be visualized by imagining a roulette wheel where each
         * individual is assigned one slice of the wheel. The size of each slice
         * is determined by the fitness of the individual where more fit
         * individuals are assigned a larger slice. Then the wheel is spun and
         * whichever slice is chosen will cause the associated individual to be
         * selected.
         * @see InitializeSelector
         * @see Population::RouletteWheelSelect
         */
        RouletteWheel,

        /**
         * Tournament selector.<br/>
         * Chooses n individuals randomly from the population and selects the one
         * with the highest fitness.<br/>
         * This tends to select individuals with higher fitness but the tournament
         * size has an effect.<br/>
         * If n is 1, tournament selector is identical to uniform selector.<br/>
         * If n is the population size, tournament selector is identical to rank
         * selector.<br/>
         * You can control n with the _tournamentSize property.
         * @see _tournamentSize
         * @see SetTournamentSize
         * @see GetTournamentSize
         * @see Population::TournamentSelect
         */
        Tournament
    };

    /**
     * Method by which the mutation rate is adjusted over the course of running
     * the genetic algorithm.
     * @see SetMutationRateSchedule
     * @see GetMutationRateSchedule
     * @see GetCurrentMutationRate
     * @see SetMutationRate
     * @see GetMutationRate
     */
    enum class MutationRateSchedule : uint8_t {
        /**
         * The mutation rate is constant and does not vary from generation to
         * generation.<br/>
         * The rate will always be _mutationRate.
         * @see _mutationRate
         * @see SetMutationRate
         * @see GetMutationRate
         */
        Constant = 1,

        /**
         * The mutation rate varies by generation.<br/>
         * Early generations introduce a lot of mutation which slowly tapers off
         * to 0 by the last generation. (last generation is controlled by
         * _totalGenerations)<br/>
         * Strictly, when the current generation has passed _totalGenerations,
         * we will set the current mutation rate to _mutationRate instead of
         * 0 because we do not want to stall the evolution.<br/>
         * The computed mutation rate has nothing to do with _mutationRate unless
         * we have run-past _totalGenerations.<br/>
         * This mutation schedule does a good job of using the mutation operator
         * to search the solution-space of the chromosome.
         * @see _totalGenerations
         * @see _mutationRate
         * @see SetMutationRate
         * @see GetMutationRate
         * @see SetTotalGenerations
         * @see GetTotalGenerations
         */
        Deterministic,

        /**
         * Attempt to increase mutation when the population converges too much.
         * Uses the population diversity score as a metric.<br/>
         * When this diveristy falls below a floor value
         * (_selfAdaptiveMutationDiversityFloor), we will introduce more aggressive
         * mutation in order to increase diversity and keep the evolution from
         * converging.<br/>
         * The more aggressive mutation rate is configurable via
         * _selfAdaptiveMutationAggressiveRate.<br/>
         * When diversity is not below the floor value, we use a constant mutation
         * rate controlled by _mutationRate.
         * @see _mutationRate
         * @see SetMutationRate
         * @see GetMutationRate
         * @see _selfAdaptiveMutationDiversityFloor
         * @see SetSelfAdaptiveMutationDiversityFloor
         * @see GetSelfAdaptiveMutationDiversityFloor
         * @see _selfAdaptiveMutationAggressiveRate
         * @see SetSelfAdaptiveMutationAggressiveRate
         * @see GetSelfAdaptiveMutationAggressiveRate
         */
        SelfAdaptive,

        /**
         * Calculates the mutation rate required to flip a number of bits in the
         * chromosome.<br/>
         * The number of bits is controlled by _proportionalMutationBitCount.
         * @see _proportionalMutationBitCount
         * @see SetProportionalMutationBitCount
         * @see GetProportionalMutationBitCount
         */
        Proportional
    };

    typedef double(*FitnessFunction)(Individual*, void*);

protected:
    const Genome* _genome;
    Population _population;
    Population _lastGenerationPopulation;
    MutationRateSchedule _mutationRateSchedule;

    size_t _populationSize;
    size_t _totalGenerations;
    size_t _currentGeneration;

    size_t _eliteCount;
    size_t _mutatedEliteCount;

    double _mutationRate;
    double _crossoverRate;
    double _mutatedEliteMutationRate;

    CrossoverType _crossoverType;
    MutatorType _mutatorType;
    SelectorType _selectorType;

    size_t _tournamentSize;
    size_t _kPointCrossoverPointCount;
    double _selfAdaptiveMutationDiversityFloor;
    double _selfAdaptiveMutationAggressiveRate;
    size_t _proportionalMutationBitCount;

    bool _crossoverIgnoreGeneBoundaries;
    bool _allowSameParentCouples;

    void* _userData;
    FitnessFunction _fitnessFunction;

    RandomWrapper _randomWrapper;

public:
    GeneticAlgorithm();
    ~GeneticAlgorithm();

    /**
     * Set the genome we will use to construct Individuals which make up the
     * populations of each generation.
     */
    void SetGenome(const Genome* genome);
    const Genome* GetGenome();

    /**
     * When we perform crossover, should we respect gene boundaries such that
     * all of the bits making up a gene are copied from one parent?<br/>
     * This could be useful if the bits underlying the gene don't make as
     * much sense when they are split up.<br/>
     * If this flag is false, we will respect the boundary of each gene and
     * not split a gene up during crossover.<br/>
     * If this flag is true, we will ignore gene boundaries during crossover
     * and treat the chromosome as a big chunk of binary data which will
     * be split according to the crossover operator chosen.<br/>
     * In general, this flag should be left at the default of true.
     */
    void SetCrossoverIgnoreGeneBoundaries(bool crossoverIgnoreGeneBoundaries);
    bool GetCrossoverIgnoreGeneBoundaries();

    /**
     * When we choose parents to use for crossover, should we allow the same
     * Individual to be used as both parents?<br/>
     * This could be useful because crossover returns the same parent if both
     * parents are identical.<br/>
     * If this flag is false, we will not allow the same Individual to be
     * selected as both parents when we select a couple for crossover.<br/>
     * If this flag is true, it is possible that both parents in the
     * couple selected for crossover will be the same Individual.
     * This flag is true by default.
     */
    void SetAllowSameParentCouples(bool allowSameParentCouples);
    bool GetAllowSameParentCouples();

    /**
     * Set the number of Individuals we should include in the tournament
     * when using tournament selector.
     */
    void SetTournamentSize(size_t tournamentSize);
    size_t GetTournamentSize();

    /**
     * Set the number of points we'll use to cut up the parent chromosomes
     * during k-point crossover.
     */
    void SetKPointCrossoverPointCount(size_t kPointCrossoverPointCount);
    size_t GetKPointCrossoverPointCount();

    /**
     * Set the floor value for the population diversity, below which we will
     * more aggressively mutate the population.
     */
    void SetSelfAdaptiveMutationDiversityFloor(double selfAdaptiveMutationDiversityFloor);
    double GetSelfAdaptiveMutationDiversityFloor();

    /**
     * Set the aggressive mutation rate we will switch to when the population
     * diversity falls below the self-adaptive mutation diversity floor.
     */
    void SetSelfAdaptiveMutationAggressiveRate(double selfAdaptiveMutationAggressiveRate);
    double GetSelfAdaptiveMutationAggressiveRate();

    /**
     * Set the number of bits we should attempt to mutate with each mutation
     * operation.
     */
    void SetProportionalMutationBitCount(size_t proportionalMutationBitCount);
    size_t GetProportionalMutationBitCount();

    /**
     * Set the total number of generations we want to evolve.<br/>
     * If you call Run(), we will execute this many generations.<br/>
     * If you manually call Step(), you can run more than total generations.
     * <br/>
     * This number is used to calculate mutation rate in some mutation
     * rate schedules.
     */
    void SetTotalGenerations(size_t totalGenerations);
    size_t GetTotalGenerations();

    /**
     * Set the mutation rate.<br/>
     * After we perform crossover, mutate each offspring Individual by this
     * rate according to the mutation operator.<br/>
     * During execution, the actual mutation rate is calculated based on the
     * mutation rate schedule.
     * @see MutatorType
     * @see MutationRateSchedule
     * @see GetCurrentMutationRate
     */
    void SetMutationRate(double mutationRate);
    double GetMutationRate();

    /**
     * Set the crossover rate.<br/>
     * During each step, construct new individuals for the next generation.<br/>
     * We will select two Individuals and perform crossover to produce the
     * offspring in the next generation with a crossover rate chance.<br/>
     * Instead of crossover, we will copy one Individual from the old
     * population over into the new one with (1 - crossover rate) chance.
     */
    void SetCrossoverRate(double crossoverRate);
    double GetCrossoverRate();

    /**
     * Set the crossover type.<br/>
     * We will use this type to crossover two parent individuals in order to
     * produce offspring for the next generation.
     * @see CrossoverType
     */
    void SetCrossoverType(CrossoverType crossoverType);
    CrossoverType GetCrossoverType();

    /**
     * Set the selector type.<br/>
     * We will use the selector specified to select two parent individuals on
     * which to perform crossover to produce offspring for the next generation.
     * @see SelectorType
     */
    void SetSelectorType(SelectorType selectorType);
    SelectorType GetSelectorType();

    /**
     * When constructing the next generation, always copy some of the best
     * Individuals from the current generation. These elite Individuals
     * are copied as they are and are not crossed-over or mutated.
     * @param eliteCount Number of elites.
     */
    void SetEliteCount(size_t eliteCount);
    size_t GetEliteCount();

    /**
     * Mutated elites are the same as ordinary elite Individuals except they
     * are subjected to the mutation operator according to
     * mutatedEliteMutationRate.
     * @see SetMutatedEliteMutationRate
     */
    void SetMutatedEliteCount(size_t mutatedEliteCount);
    size_t GetMutatedEliteCount();

    /**
     * Set the rate at mutated elite individuals will be mutated.
     */
    void SetMutatedEliteMutationRate(double mutatedEliteMutationRate);
    double GetMutatedEliteMutationRate();

    /**
     * Set the mutation rate schedule we will use to determine the mutation
     * rate over the course of many generations.
     * @see MutationRateSchedule
     */
    void SetMutationRateSchedule(MutationRateSchedule mutationRateSchedule);
    MutationRateSchedule GetMutationRateSchedule();

    /**
     * Set the mutator type.<br/>
     * We will use the mutator to mutate each new offspring before adding it
     * to the next generation population.
     * @see MutatorType
     */
    void SetMutatorType(MutatorType mutatorType);
    MutatorType GetMutatorType();

    /**
     * Set some data which will be passed into the fitness function called to
     * score each Individual.
     */
    void SetUserData(void* userData);
    void* GetUserData();

    /**
     * Set the fitness function used to evaluate the fitness of each Individual.
     * <br/>
     * The fitness function takes a pointer to an Individual and a void* which
     * is the user data previously set via GeneticAlgorithm::SetUserData.
     */
    void SetFitnessFunction(FitnessFunction fitnessFunction);
    FitnessFunction GetFitnessFunction();

    /**
     * Each generation, we will construct and evaluate populationSize
     * Individuals.
     */
    void SetPopulationSize(size_t populationSize);
    size_t GetPopulationSize();

    /**
     * Return the current generation.<br/>
     * The generation increases after every step.
     */
    size_t GetCurrentGeneration();

    /**
     * Return the best Individual from the current population.
     */
    Individual* GetBestIndividual();

    /**
     * Return the Individual at index position from the current population.
     */
    Individual* GetIndividual(size_t index);

    /**
     * Get the minimum score among individuals in the current population.
     */
    double GetMinimumScore();

    /**
     * Get the average score between individuals in the current population.
     */
    double GetAverageScore();

    /**
     * Get the standard deviation of scores between individuals in the current
     * population.
     */
    double GetScoreStandardDeviation();

    /**
     * Get the population diversity among individuals in the current population.
     * <br/>
     * The population diversity is a metric used to indicate how much the
     * genetic material backing the individuals varies. A high diversity value
     * means the individuals have very different genetic components. A value
     * of zero means the individuals are identical.
     */
    double GetPopulationDiversity();

    /**
     * Initialize the state of the GeneticAlgorithm.<br/>
     * If initialPopulation is passed, use these bits as the chromosomes for
     * our new population. Otherwise, create a population filled with random
     * individuals.<br/>
     * @param initialPopulation We will construct new population members and
     * interpret these values as binary chromosome data. Pass nullptr to
     * initialize the population randomly.
     */
    void Initialize(const std::vector<const BitVector*>* initialPopulation = nullptr);

    /**
     * Perform one step of the genetic algorithm:<br/>
     *   1. Score the current population of individuals.<br/>
     *      * This will either be the initial population (if current generation
     *        is 0) or the population we constructed during the previous step
     *        operation.<br/>
     *      * The score operation will leave the population sorted.<br/>
     *   2. Use the current population to construct the next generation
     *      population.<br/>
     *      * Uses a combination of elitism, crossover, and mutation to
     *        construct the new population.<br/>
     *      * Individuals are chosen from the current population based on their
     *        fitness scores.<br/>
     *      * The next generation population is not scored, yet. It will be
     *        during the next step operation.<br/>
     *   3. Advance the current generation.
     */
    void Step();

    /**
     * Run the GeneticAlgorithm totalGenerations steps.
     */
    void Run();

protected:
    GeneticAlgorithm(const GeneticAlgorithm&);

    /**
     * Delete each Individual in the population and reset the population
     * size to 0.
     */
    void DeletePopulation(Population* population);

    /**
     * Score each Individual in the population and then sort the population
     * in terms of decreasing fitness.
     */
    void Evaluate(Population* population);

    /**
     * Use the fitness function to score one Individual.
     */
    void Score(Individual* individual);

    template <bool ignoreGeneBoundaries>
    void CrossoverInternal(Individual* parent1, Individual* parent2, Individual* offspring);

    void Crossover(Individual* parent1, Individual* parent2, Individual* offspring);
    void Mutate(Individual* individual, double mutationPercentage);
    double GetCurrentMutationRate();

    void InitializeSelector(Population* population);
    void SelectParents(Population* population, std::pair<Individual*, Individual*>* parents);
    Individual* SelectOne(Population* population);
};

} // namespace panga
