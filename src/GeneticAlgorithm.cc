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
    _crossoverType(CrossoverType::TwoPoint),
    _mutatorType(MutatorType::Flip),
    _selectorType(SelectorType::RouletteWheel),
    _tournamentSize(2),
    _crossoverIgnoreGeneBoundaries(true),
    _allowSameParentCouples(false),
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

void GeneticAlgorithm::SetCrossoverRate(double crossoverRate) {
    this->_crossoverRate = crossoverRate;
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

void GeneticAlgorithm::SetUserData(void* userData) {
    this->_userData = userData;
}

void* GeneticAlgorithm::GetUserData() {
    return this->_userData;
}

void GeneticAlgorithm::SetCrossoverType(CrossoverType crossoverType) {
    this->_crossoverType = crossoverType;
}

void GeneticAlgorithm::SetMutatorType(MutatorType mutatorType) {
    this->_mutatorType = mutatorType;
}

void GeneticAlgorithm::SetSelectorType(SelectorType selectorType) {
    this->_selectorType = selectorType;
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

Individual* GeneticAlgorithm::GetBestIndividual() {
    return this->_population.front();
}

Individual* GeneticAlgorithm::GetIndividual(size_t index) {
    assert(index < this->_population.size());
    return this->_population[index];
}

double GeneticAlgorithm::GetMinScore() {
    assert(!this->_population.empty());
    return this->_population.front()->GetScore();
}

double GeneticAlgorithm::GetAverageScore() {
    assert(this->_population.size() == this->_populationSize);

    if (this->_population.empty()) {
        return 0.0;
    }

    double sum = 0.0;
    std::for_each(this->_population.begin(), this->_population.end(), [&] (const Individual* i) {
        sum += i->GetScore();
    });
    return sum / this->_population.size();
}

double GeneticAlgorithm::GetScoreStdev() {
    assert(this->_population.size() == this->_populationSize);

    if (this->_population.size() <= 1) {
        return 0.0;
    }

    double mean = this->GetAverageScore();
    double sum = 0.0;
    std::for_each(this->_population.begin(), this->_population.end(), [&](const Individual* i) {
        double score = i->GetScore();
        sum += (score - mean) * (score - mean);
    });
    return std::sqrt(sum / (this->_population.size() - 1));
}

double GeneticAlgorithm::GetPopulationDiversity() {
    uint64_t distance = 0;

    for (size_t i = 0; i < this->_populationSize; i++) {
        for (size_t j = i + 1; j < this->_populationSize; j++) {
            distance += this->_population.at(i)->HammingDistance(this->_population.at(j));
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

void GeneticAlgorithm::Initialize(std::vector<BitVector*>* initialPopulation) {
    // Delete any existing population members.
    DeletePopulation(&this->_population);
    DeletePopulation(&this->_lastGenerationPopulation);

    // Create new population members based on the initialPopulation.
    if (initialPopulation != nullptr) {
        for (std::vector<BitVector*>::iterator it = initialPopulation->begin(); it != initialPopulation->end(); ++it) {
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

    this->Evaluate();
}

void GeneticAlgorithm::Step() {
    assert(this->_population.size() == this->_lastGenerationPopulation.size());

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

    // Score and sort the new population.
    this->Evaluate();

    this->_currentGeneration++;
}

void GeneticAlgorithm::Score(Individual* individual) {
    individual->SetScore(this->_fitnessFunction(individual, this->_userData));
}

void GeneticAlgorithm::Evaluate() {
    assert(this->_population.size() == this->_populationSize);

    // Score current population.
    std::for_each(this->_population.begin(), this->_population.end(), [&](Individual* i) {
        this->Score(i);
    });

    // Sort the population by increasing raw score.
    std::sort(this->_population.begin(), this->_population.end(),
        [](Individual* const& a, Individual* const& b) { return *a < *b; });

    // Calculate the Individual fitness scores.
    double scoreSum = 0.0;
    double bestScore = this->_population.front()->GetScore();
    double worstScore = this->_population.back()->GetScore();

    // We need to invert the trend of scores.
    // First, calculate best+worst - score[i] as an intermediate score.
    std::for_each(this->_population.begin(), this->_population.end(), [&](Individual* i) {
        double tempScore = bestScore + worstScore - i->GetScore();
        scoreSum += tempScore;
        i->SetFitness(tempScore);
    });

    // Now calculate fitness as a proportion of the intermediate sum.
    std::for_each(this->_population.begin(), this->_population.end(), [&](Individual* i) {
        i->SetFitness(i->GetFitness() / scoreSum);
    });
}

void GeneticAlgorithm::Run() {
    this->Initialize();

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
        size_t n = this->_genome->BitsRequired();
        return 1.0 / (2.0 + ((n - 2.0) / (this->_totalGenerations - 1.0)) * this->_currentGeneration);
    }
    case MutationRateSchedule::SelfAdaptive:
        if (this->GetPopulationDiversity() < 0.250) {
            return 0.20;
        } else {
            return this->_mutationRate;
        }
    case MutationRateSchedule::Proportional:
        // Proportional schedule sets a mutation probability in order to flip mutationRate bits.
        assert(this->_genome->BitsRequired() != 0);
        return this->_mutationRate * (1.0 / this->_genome->BitsRequired());
    default:
        // Invalid mutation rate schedule
        assert(false);
        return 0.0;
    }
}

void GeneticAlgorithm::Crossover(Individual* parent1, Individual* parent2, Individual* offspring) {
    assert(parent1->GetBitCount() == parent2->GetBitCount());

    switch (this->_crossoverType) {
    case CrossoverType::OnePoint:
        if (this->_crossoverIgnoreGeneBoundaries) {
            Chromosome::KPointCrossover<1, true>(parent1, parent2, offspring, &this->_randomWrapper);
        } else {
            Chromosome::KPointCrossover<1, false>(parent1, parent2, offspring, &this->_randomWrapper);
        }
        break;
    case CrossoverType::TwoPoint:
        if (this->_crossoverIgnoreGeneBoundaries) {
            Chromosome::KPointCrossover<2, true>(parent1, parent2, offspring, &this->_randomWrapper);
        } else {
            Chromosome::KPointCrossover<2, false>(parent1, parent2, offspring, &this->_randomWrapper);
        }
        break;
    case CrossoverType::Uniform:
        if (this->_crossoverIgnoreGeneBoundaries) {
            Chromosome::UniformCrossover<true>(parent1, parent2, offspring, &this->_randomWrapper);
        } else {
            Chromosome::UniformCrossover<false>(parent1, parent2, offspring, &this->_randomWrapper);
        }
        break;
    default:
        assert(false);
        break;
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
