//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "Individual.h"

#include <algorithm>
#include <cassert>

#include "Genome.h"

namespace panga {

Individual::Individual(const Genome& genome) : Chromosome(genome) {}

Individual::Individual(const Genome& genome, const BitVector& chromosome)
    : Chromosome(genome) {
  // Chromosome source vector must have the same number of bits as our genome.
  assert(chromosome.GetBitCount() == genome.BitsRequired());
  // Copy bits from existing chromosome.
  BitVector::operator=(chromosome);
}

Individual& Individual::operator=(const Individual& rhs) {
  if (this != &rhs) {
    score_ = rhs.score_;
    fitness_ = rhs.fitness_;
    BitVector::operator=(rhs);
  }
  return *this;
}

bool Individual::operator<(const Individual& rhs) const {
  return score_ < rhs.score_;
}

double Individual::GetFitness() const { return fitness_; }

double Individual::GetScore() const { return score_; }

void Individual::SetFitness(double fitness) { fitness_ = fitness; }

void Individual::SetScore(double score) { score_ = score; }

}  // namespace panga
