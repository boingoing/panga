#pragma once

#include <vector>

#include "RandomWrapper.h"
#include "Population.h"

namespace panga {

class Genome;
class Individual;
class BitVector;

enum class CrossoverType : uint8_t {
    OnePoint = 1,
    TwoPoint,
    Uniform
};

enum class MutatorType : uint8_t {
    Flip = 1
};

enum class SelectorType : uint8_t {
    Rank = 1,
    Uniform,
    RouletteWheel,
    Tournament
};

enum class MutationRateSchedule : uint8_t {
    Constant = 1,
    Deterministic,
    SelfAdaptive,
    Proportional
};

class GeneticAlgorithm {
    typedef std::pair<Individual*, Individual*> Parents;
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

    bool _crossoverIgnoreGeneBoundaries;
    bool _allowSameParentCouples;

    void* _userData;
    FitnessFunction _fitnessFunction;

    RandomWrapper _randomWrapper;

public:
    GeneticAlgorithm();
    ~GeneticAlgorithm();

    void SetGenome(const Genome* genome);
    const Genome* GetGenome();

    /**
     * When we perform crossover, should we respect gene boundaries such that
     * all of the bits making up a gene are copied from one parent? This could
     * be useful if the bits underlying the gene don't make as much sense
     * when they are split up.
     * If this flag is false, we will respect the boundary of each gene and
     * not split a gene up during crossover.
     * If this flag is true, we will ignore gene boundaries during crossover
     * and treat the chromosome as a big chunk of binary data which will
     * be split according to the crossover operator chosen.
     * In general, this flag should be left at the default of true.
     */
    void SetCrossoverIgnoreGeneBoundaries(bool crossoverIgnoreGeneBoundaries);
    bool GetCrossoverIgnoreGeneBoundaries();

    void SetAllowSameParentCouples(bool allowSameParentCouples);
    bool GetAllowSameParentCouples();

    void SetTournamentSize(size_t tournamentSize);
    size_t GetTournamentSize();

    void SetTotalGenerations(size_t totalGenerations);
    size_t GetTotalGenerations();

    void SetMutationRate(double mutationRate);
    void SetCrossoverRate(double crossoverRate);
    void SetCrossoverType(CrossoverType crossoverType);
    void SetSelectorType(SelectorType selectorType);

    void SetEliteCount(size_t eliteCount);
    size_t GetEliteCount();

    void SetMutatedEliteCount(size_t mutatedEliteCount);
    size_t GetMutatedEliteCount();

    void SetMutatedEliteMutationRate(double mutatedEliteMutationRate);
    double GetMutatedEliteMutationRate();

    void SetMutationRateSchedule(MutationRateSchedule mutationRateSchedule);
    void SetMutatorType(MutatorType mutatorType);

    /**
     * Set some data which will be passed into the fitness function called to
     * score each Individual.
     */
    void SetUserData(void* userData);
    void* GetUserData();

    /**
     * Set the fitness function used to evaluate the fitness of each Individual.
     * The fitness function takes a pointer to an Individual and a void* which
     * is the user data previously set via GeneticAlgorithm::SetUserData.
     */
    void SetFitnessFunction(FitnessFunction fitnessFunction);
    FitnessFunction GetFitnessFunction();

    void SetPopulationSize(size_t populationSize);
    size_t GetPopulationSize();

    /**
     * Return the current generation.
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
     * The population diversity is a metric used to indicate how much the
     * genetic material backing the individuals varies. A high diversity value
     * means the individuals have very different genetic components. A value
     * of zero means the individuals are identical.
     */
    double GetPopulationDiversity();

    /**
     * Initialize the state of the GeneticAlgorithm.
     * If initialPopulation is passed, use these bits as the chromosomes for
     * our new population. Otherwise, create a population filled with random
     * individuals.
     * @param initialPopulation We will construct new population members and
     * interpret these values as binary chromosome data. Pass nullptr to
     * initialize the population randomly.
     */
    void Initialize(const std::vector<const BitVector*>* initialPopulation = nullptr);

    /**
     * Perform one step of the genetic algorithm:
     *   1) Score the current population of individuals.
     *      a) This will either be the initial population (if current generation
     *         is 0) or the population we constructed during the previous step
     *         operation.
     *      b) The score operation will leave the population sorted.
     *   2) Use the current population to construct the next generation
     *      population.
     *      a) Uses a combination of elitism, crossover, and mutation to
     *         construct the new population.
     *      b) Individuals are chosen from the current population based on their
     *         fitness scores.
     *      c) The next generation population is not scored, yet. It will be
     *         during the next step operation.
     *   3) Advance the current generation.
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

    void Crossover(Individual* parent1, Individual* parent2, Individual* offspring);
    void Mutate(Individual* individual, double mutationPercentage);
    double GetCurrentMutationRate();

    void InitializeSelector(Population* population);
    void SelectParents(Population* population, Parents* parents);
    Individual* SelectOne(Population* population);
};

} // namespace panga
