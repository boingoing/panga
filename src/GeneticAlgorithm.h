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

    Individual* GetBestIndividual();
    Individual* GetIndividual(size_t index);

    double GetMinScore();
    double GetAverageScore();
    double GetScoreStdev();
    double GetPopulationDiversity();

    void Initialize(std::vector<BitVector*>* initialPopulation = nullptr);
    void Step();
    void Evaluate();

    /**
     * Use the fitness function to score one Individual.
     */
    void Score(Individual* individual);

    /**
     * Run the GeneticAlgorithm totalGenerations steps.
     * Also performs initialization.
     */
    void Run();

    double GetCurrentMutationRate();

protected:
    GeneticAlgorithm(const GeneticAlgorithm&);

    void DeletePopulation(Population* population);

    void Crossover(Individual* parent1, Individual* parent2, Individual* offspring);
    void Mutate(Individual* individual, double mutationPercentage);

    void InitializeSelector(Population* population);
    void SelectParents(Population* population, Parents* parents);
    Individual* SelectOne(Population* population);
};

} // namespace panga
