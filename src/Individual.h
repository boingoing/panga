//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef INDIVIDUAL_H__
#define INDIVIDUAL_H__

#include "Chromosome.h"

namespace panga {

class BitVector;
class Genome;

/**
 * An Individual is a Chromosome with a fitness score.
 */
class Individual : public Chromosome {
public:
    explicit Individual(const Genome& genome);
    Individual(const Genome& genome, const BitVector& chromosome);
    Individual(const Individual& rhs) = delete;
    ~Individual() = default;

    /**
     * Copy |rhs| into this Individual.
     */
    Individual& operator=(const Individual& rhs);

    /**
     * Less than operator, return negative if this score is less than rhs.score.
     */
    bool operator<(const Individual& rhs) const;

    /**
     * Get the score value returned from the fitness function when we called it
     * with this Individual.<br/>
     * A lower raw score indicates a more fit Individual.
     */
    double GetScore() const;
    void SetScore(double score);

    /**
     * Get the proportional fitness score calculated after we've scored all
     * Individuals in the population with this one.<br/>
     * A higher proportional fitness score indicates a more fit Individual.
     */
    double GetFitness() const;
    void SetFitness(double fitness);

private:
    /**
     * Proportional fitness score.
     */
    double fitness_ = 0.0;

    /**
     * Raw score received from fitness function.
     */
    double score_ = 0.0;
};

}  // namespace panga

#endif  // INDIVIDUAL_H__
