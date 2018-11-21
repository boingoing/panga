//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include "Chromosome.h"

namespace panga {

class BitVector;
class Genome;

class Individual : public Chromosome {
protected:
    /**
     * Proportional fitness score.
     */
    double _fitness;

    /**
     * Raw score received from fitness function.
     */
    double _score;

public:
    Individual(const Genome* genome, const BitVector* chromosome = nullptr);
    ~Individual();

    /**
     * Less than operator, return negative if this score is less than rhs.score.
     */
    bool operator<(const Individual& rhs) const;

    double GetFitness() const;
    double GetScore() const;

    void SetFitness(double fitness);
    void SetScore(double score);
};

} // namespace panga
