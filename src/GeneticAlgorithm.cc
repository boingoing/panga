#include <utility>
#include <algorithm>
#include <numeric>

#include "GeneticAlgorithm.h"
#include "Individual.h"

using namespace panga;

GeneticAlgorithm::GeneticAlgorithm() :
    _genome(nullptr),
    _mutationRateSchedule(MutationRateSchedule::Constant),
    _populationSize(0),
    _totalGenerations(0),
    _currentGeneration(0),
    _eliteCount(0),
    _mutatedEliteCount(0),
    _mutationRate(0.05),
    _crossoverRate(0.9),
    _mutatedEliteMutationRate(0.0),
    _crossoverType(CrossoverType::Uniform),
    _mutatorType(MutatorType::Flip),
    _selectorType(SelectorType::Tournament),
    _tournamentSize(2),
    _kPointCrossoverPointCount(3),
    _selfAdaptiveMutationDiversityFloor(0.0002),
    _selfAdaptiveMutationAggressiveRate(0.1),
    _proportionalMutationBitCount(1),
    _crossoverIgnoreGeneBoundaries(true),
    _allowSameParentCouples(true),
    _userData(nullptr),
    _fitnessFunction(nullptr) {
}

GeneticAlgorithm::~GeneticAlgorithm() {
    DeletePopulation(&this->_population);
    DeletePopulation(&this->_lastGenerationPopulation);
}

void GeneticAlgorithm::SetGenome(const Genome* genome) {
    this->_genome = genome;
}

const Genome* GeneticAlgorithm::GetGenome() {
    return this->_genome;
}

void GeneticAlgorithm::SetMutatedEliteMutationRate(double mutatedEliteMutationRate) {
    this->_mutatedEliteMutationRate = mutatedEliteMutationRate;
}

double GeneticAlgorithm::GetMutatedEliteMutationRate() {
    return this->_mutatedEliteMutationRate;
}

void GeneticAlgorithm::SetMutationRate(double mutationRate) {
    this->_mutationRate = mutationRate;
}

double GeneticAlgorithm::GetMutationRate() {
    return this->_mutationRate;
}

void GeneticAlgorithm::SetCrossoverRate(double crossoverRate) {
    this->_crossoverRate = crossoverRate;
}

double GeneticAlgorithm::GetCrossoverRate() {
    return this->_crossoverRate;
}

void GeneticAlgorithm::SetPopulationSize(size_t populationSize) {
    this->_populationSize = populationSize;
}

size_t GeneticAlgorithm::GetPopulationSize() {
    return this->_populationSize;
}

void GeneticAlgorithm::SetTotalGenerations(size_t totalGenerations) {
    this->_totalGenerations = totalGenerations;
}

size_t GeneticAlgorithm::GetTotalGenerations() {
    return this->_totalGenerations;
}

void GeneticAlgorithm::SetFitnessFunction(FitnessFunction fitnessFunction) {
    this->_fitnessFunction = fitnessFunction;
}

GeneticAlgorithm::FitnessFunction GeneticAlgorithm::GetFitnessFunction() {
    return this->_fitnessFunction;
}

size_t GeneticAlgorithm::GetEliteCount() {
    return this->_eliteCount;
}

void GeneticAlgorithm::SetEliteCount(size_t eliteCount) {
    this->_eliteCount = eliteCount;
}

size_t GeneticAlgorithm::GetMutatedEliteCount() {
    return this->_mutatedEliteCount;
}

void GeneticAlgorithm::SetMutatedEliteCount(size_t mutatedEliteCount) {
    this->_mutatedEliteCount = mutatedEliteCount;
}

void GeneticAlgorithm::SetMutationRateSchedule(MutationRateSchedule mutationRateSchedule) {
    this->_mutationRateSchedule = mutationRateSchedule;
}

MutationRateSchedule GeneticAlgorithm::GetMutationRateSchedule() {
    return this->_mutationRateSchedule;
}

void GeneticAlgorithm::SetUserData(void* userData) {
    this->_userData = userData;
}

void* GeneticAlgorithm::GetUserData() {
    return this->_userData;
}

void GeneticAlgorithm::SetCrossoverType(CrossoverType crossoverType) {
    this->_crossoverType = crossoverType;
}

CrossoverType GeneticAlgorithm::GetCrossoverType() {
    return this->_crossoverType;
}

void GeneticAlgorithm::SetMutatorType(MutatorType mutatorType) {
    this->_mutatorType = mutatorType;
}

MutatorType GeneticAlgorithm::GetMutatorType() {
    return this->_mutatorType;
}

void GeneticAlgorithm::SetSelectorType(SelectorType selectorType) {
    this->_selectorType = selectorType;
}

SelectorType GeneticAlgorithm::GetSelectorType() {
    return this->_selectorType;
}

void GeneticAlgorithm::SetCrossoverIgnoreGeneBoundaries(bool crossoverIgnoreGeneBoundaries) {
    this->_crossoverIgnoreGeneBoundaries = crossoverIgnoreGeneBoundaries;
}

bool GeneticAlgorithm::GetCrossoverIgnoreGeneBoundaries() {
    return this->_crossoverIgnoreGeneBoundaries;
}

void GeneticAlgorithm::SetAllowSameParentCouples(bool allowSameParentCouples) {
    this->_allowSameParentCouples = allowSameParentCouples;
}

bool GeneticAlgorithm::GetAllowSameParentCouples() {
    return this->_allowSameParentCouples;
}

size_t GeneticAlgorithm::GetCurrentGeneration() {
    return this->_currentGeneration;
}

void GeneticAlgorithm::SetTournamentSize(size_t tournamentSize) {
    this->_tournamentSize = tournamentSize;
}

size_t GeneticAlgorithm::GetTournamentSize() {
    return this->_tournamentSize;
}

void GeneticAlgorithm::SetKPointCrossoverPointCount(size_t kPointCrossoverPointCount) {
    this->_kPointCrossoverPointCount = kPointCrossoverPointCount;
}

size_t GeneticAlgorithm::GetKPointCrossoverPointCount() {
    return this->_kPointCrossoverPointCount;
}

void GeneticAlgorithm::SetSelfAdaptiveMutationDiversityFloor(double selfAdaptiveMutationDiversityFloor) {
    this->_selfAdaptiveMutationDiversityFloor = selfAdaptiveMutationDiversityFloor;
}

double GeneticAlgorithm::GetSelfAdaptiveMutationDiversityFloor() {
    return this->_selfAdaptiveMutationDiversityFloor;
}

void GeneticAlgorithm::SetSelfAdaptiveMutationAggressiveRate(double selfAdaptiveMutationAggressiveRate) {
    this->_selfAdaptiveMutationAggressiveRate = selfAdaptiveMutationAggressiveRate;
}

double GeneticAlgorithm::GetSelfAdaptiveMutationAggressiveRate() {
    return this->_selfAdaptiveMutationAggressiveRate;
}

void GeneticAlgorithm::SetProportionalMutationBitCount(size_t proportionalMutationBitCount) {
    this->_proportionalMutationBitCount = proportionalMutationBitCount;
}

size_t GeneticAlgorithm::GetProportionalMutationBitCount() {
    return this->_proportionalMutationBitCount;
}

Individual* GeneticAlgorithm::GetBestIndividual() {
    if (this->_lastGenerationPopulation.empty()) {
        return nullptr;
    }

    return this->_lastGenerationPopulation.front();
}

Individual* GeneticAlgorithm::GetIndividual(size_t index) {
    assert(index < this->_lastGenerationPopulation.size());

    return this->_lastGenerationPopulation[index];
}

double GeneticAlgorithm::GetMinimumScore() {
    assert(!this->_lastGenerationPopulation.empty());

    return this->_lastGenerationPopulation.front()->GetScore();
}

double GeneticAlgorithm::GetAverageScore() {
    if (this->_lastGenerationPopulation.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    std::for_each(this->_lastGenerationPopulation.begin(), this->_lastGenerationPopulation.end(), [&] (const Individual* i) {
        sum += i->GetScore();
    });
    return sum / this->_lastGenerationPopulation.size();
}

double GeneticAlgorithm::GetScoreStandardDeviation() {
    if (this->_lastGenerationPopulation.size() <= 1) {
        return 0.0;
    }

    double mean = this->GetAverageScore();
    double sum = 0.0;
    std::for_each(this->_lastGenerationPopulation.begin(), this->_lastGenerationPopulation.end(), [&](const Individual* i) {
        double score = i->GetScore();
        sum += (score - mean) * (score - mean);
    });
    return std::sqrt(sum / (this->_lastGenerationPopulation.size() - 1));
}

double GeneticAlgorithm::GetPopulationDiversity() {
    assert(this->_lastGenerationPopulation.size() == this->_populationSize);

    size_t distance = 0;

    for (size_t i = 0; i < this->_populationSize; i++) {
        for (size_t j = i + 1; j < this->_populationSize; j++) {
            distance += this->_lastGenerationPopulation.at(i)->HammingDistance(this->_lastGenerationPopulation.at(j));
        }
    }

    return ((double)distance) / ((double)this->_genome->BitsRequired() * ((double)((this->_populationSize*(this->_populationSize - 1)) >> 2)));
}

void GeneticAlgorithm::DeletePopulation(Population* population) {
    std::for_each(population->begin(), population->end(), [](Individual* i) {
        delete i;
    });
    population->clear();
}

void GeneticAlgorithm::Initialize(const std::vector<const BitVector*>* initialPopulation) {
    assert(this->_genome != nullptr);

    // Delete any existing population members.
    DeletePopulation(&this->_population);
    DeletePopulation(&this->_lastGenerationPopulation);

    // Create new population members based on the initialPopulation.
    if (initialPopulation != nullptr) {
        for (std::vector<const BitVector*>::const_iterator it = initialPopulation->begin(); it != initialPopulation->end(); ++it) {
            if (this->_population.size() < this->_populationSize) {
                this->_population.push_back(new Individual(this->_genome, *it));
            }
        }
    }

    // Fill out the rest of the population with random individuals.
    while (this->_population.size() < this->_populationSize) {
        Individual* i = new Individual(this->_genome);
        i->Randomize(&this->_randomWrapper);
        this->_population.push_back(i);
    }

    // We need objects to exist in the last generation, though they won't be
    // used until we perform the next step.
    while (this->_lastGenerationPopulation.size() < this->_populationSize) {
        this->_lastGenerationPopulation.push_back(new Individual(this->_genome));
    }

    // Reset the current generation.
    this->_currentGeneration = 0;
}

void GeneticAlgorithm::Step() {
    assert(this->_population.size() == this->_lastGenerationPopulation.size());

    // We are about to score the current population.
    this->_currentGeneration++;

    // Score and sort the current population.
    // This population is either the result of Initialize() or a previous
    // Step() operation.
    this->Evaluate(&this->_population);

    // Get the mutation rate for the current generation.
    double currentMutationRate = this->GetCurrentMutationRate();

    // Save current population into last generation population.
    this->_population.swap(this->_lastGenerationPopulation);

    // Initialize the selector.
    this->InitializeSelector(&this->_lastGenerationPopulation);
    auto parents = std::make_pair<Individual*, Individual*>(nullptr, nullptr);

    size_t currentNewIndividual = 0;

    // Elitism
    // Add the best individuals from last generation into the current population.
    for (size_t i = 0; i < this->_eliteCount; i++) {
        *this->_population[currentNewIndividual++] = *this->_lastGenerationPopulation[i];
    }

    // Mutated elitism
    // Take the best individuals from the last generation but mutate them by a variable rate.
    for (size_t i = 0; i < this->_mutatedEliteCount; i++) {
        Individual* newIndividual = this->_population[currentNewIndividual++];
        *newIndividual = *this->_lastGenerationPopulation[i];
        this->Mutate(newIndividual, this->_mutatedEliteMutationRate);
    }

    // Create offspring from individuals in last generation.
    while (currentNewIndividual < this->_populationSize) {
        Individual* offspring = this->_population[currentNewIndividual++];

        // Select a couple from the last generation.
        this->SelectParents(&this->_lastGenerationPopulation, &parents);

        // See if we will do crossover or duplicate a parent.
        if (this->_randomWrapper.CoinFlip(this->_crossoverRate)) {
            this->Crossover(parents.first, parents.second, offspring);
        } else {
            *offspring = *parents.first;
        }

        // Mutate offspring.
        this->Mutate(offspring, currentMutationRate);
    }
}

void GeneticAlgorithm::Score(Individual* individual) {
    individual->SetScore(this->_fitnessFunction(individual, this->_userData));
}

void GeneticAlgorithm::Evaluate(Population* population) {
    // Score current population.
    std::for_each(population->begin(), population->end(), [&](Individual* i) {
        this->Score(i);
    });

    // Sort the population by increasing raw score.
    std::sort(population->begin(), population->end(),
        [](Individual* const& a, Individual* const& b) { return *a < *b; });

    // Calculate the Individual fitness scores.
    double scoreSum = 0.0;
    double bestScore = population->front()->GetScore();
    double worstScore = population->back()->GetScore();

    // We need to invert the trend of scores.
    // First, calculate best+worst - score[i] as an intermediate score.
    std::for_each(population->begin(), population->end(), [&](Individual* i) {
        double tempScore = bestScore + worstScore - i->GetScore();
        scoreSum += tempScore;
        i->SetFitness(tempScore);
    });

    // Now calculate fitness as a proportion of the intermediate sum.
    std::for_each(population->begin(), population->end(), [&](Individual* i) {
        i->SetFitness(i->GetFitness() / scoreSum);
    });
}

void GeneticAlgorithm::Run() {
    while (this->_currentGeneration < this->_totalGenerations) {
        this->Step();
    }
}

double GeneticAlgorithm::GetCurrentMutationRate() {
    switch (this->_mutationRateSchedule) {
    case MutationRateSchedule::Constant:
        return this->_mutationRate;
    case MutationRateSchedule::Deterministic:
    {
        // If total generations is 0 or if we are on a generation after the total counter,
        // deterministic calculation would return 0 for mutation rate.
        // In that case just use the constant-schedule mutation rate instead.
        return this->_totalGenerations == 0 || this->_currentGeneration > this->_totalGenerations ?
            this->_mutationRate :
            1.0 / (2.0 + ((this->_genome->BitsRequired() - 2.0) / (this->_totalGenerations - 1.0)) * this->_currentGeneration);
    }
    case MutationRateSchedule::SelfAdaptive:
        if (this->GetPopulationDiversity() < this->_selfAdaptiveMutationDiversityFloor) {
            return this->_selfAdaptiveMutationAggressiveRate;
        } else {
            return this->_mutationRate;
        }
    case MutationRateSchedule::Proportional:
        // Proportional schedule sets a mutation probability in order to flip mutationRate bits.
        assert(this->_genome->BitsRequired() != 0);
        return this->_proportionalMutationBitCount * (1.0 / this->_genome->BitsRequired());
    default:
        // Invalid mutation rate schedule
        assert(false);
        return 0.0;
    }
}

template <bool ignoreGeneBoundaries>
void GeneticAlgorithm::CrossoverInternal(Individual* parent1, Individual* parent2, Individual* offspring) {
    switch (this->_crossoverType) {
    case CrossoverType::OnePoint:
        Chromosome::KPointCrossover<ignoreGeneBoundaries>(1, parent1, parent2, offspring, &this->_randomWrapper);
        break;
    case CrossoverType::TwoPoint:
        Chromosome::KPointCrossover<ignoreGeneBoundaries>(2, parent1, parent2, offspring, &this->_randomWrapper);
        break;
    case CrossoverType::KPoint:
        Chromosome::KPointCrossover<ignoreGeneBoundaries>(this->_kPointCrossoverPointCount, parent1, parent2, offspring, &this->_randomWrapper);
        break;
    case CrossoverType::Uniform:
        Chromosome::UniformCrossover<ignoreGeneBoundaries>(parent1, parent2, offspring, &this->_randomWrapper);
        break;
    default:
        assert(false);
    }
}

void GeneticAlgorithm::Crossover(Individual* parent1, Individual* parent2, Individual* offspring) {
    assert(parent1->GetBitCount() == parent2->GetBitCount());

    if (this->_crossoverIgnoreGeneBoundaries) {
        this->CrossoverInternal<true>(parent1, parent2, offspring);
    } else {
        this->CrossoverInternal<false>(parent1, parent2, offspring);
    }
}

void GeneticAlgorithm::Mutate(Individual* individual, double mutationPercentage) {
    switch (this->_mutatorType) {
    case MutatorType::Flip:
        Chromosome::FlipMutator(individual, mutationPercentage, &this->_randomWrapper);
        break;
    default:
        assert(false);
        break;
    }
}

void GeneticAlgorithm::InitializeSelector(Population* population) {
    if (this->_selectorType == SelectorType::RouletteWheel) {
        population->InitializePartialSums();
    }
}

Individual* GeneticAlgorithm::SelectOne(Population* population) {
    switch (this->_selectorType) {
    case SelectorType::Rank:
        return population->RankSelect();
    case SelectorType::Uniform:
        return population->UniformSelect(&this->_randomWrapper);
    case SelectorType::RouletteWheel:
        return population->RouletteWheelSelect(&this->_randomWrapper);
    case SelectorType::Tournament:
        return population->TournamentSelect(this->_tournamentSize, &this->_randomWrapper);
    default:
        assert(false);
        return nullptr;
    }
}

void GeneticAlgorithm::SelectParents(Population* population, Parents* parents) {
    assert(population->size() > 0);

    parents->first = this->SelectOne(population);

    if (this->_allowSameParentCouples) {
        parents->second = this->SelectOne(population);
    } else {
        Population temp;
        std::for_each(population->begin(), population->end(), [&](Individual* i) {
            if (i != parents->first) {
                temp.push_back(i);
            }
        });
        this->InitializeSelector(&temp);
        parents->second = this->SelectOne(&temp);
    }
}
