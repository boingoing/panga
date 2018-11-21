#include <algorithm>
#include <cassert>

#include "Individual.h"
#include "Genome.h"

using namespace panga;

Individual::Individual(const Genome* genome, const BitVector* chromosome) :
    Chromosome(genome),
    _fitness(0.0),
    _score(0.0) {
    if (chromosome != nullptr) {
        // Chromosome source vector must have the same number of bits as our genome.
        assert(chromosome->GetBitCount() == genome->BitsRequired());

        // Copy bits from existing chromosome.
        BitVector::operator=(*chromosome);
    }
}

Individual::~Individual() {
}

bool Individual::operator<(const Individual& rhs) const {
    return this->_score < rhs._score;
}

double Individual::GetFitness() const {
    return this->_fitness;
}

double Individual::GetScore() const {
    return this->_score;
}

void Individual::SetFitness(double fitness) {
    this->_fitness = fitness;
}

void Individual::SetScore(double score) {
    this->_score = score;
}
