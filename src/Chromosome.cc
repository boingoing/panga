//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>

#include "Chromosome.h"
#include "Genome.h"
#include "RandomWrapper.h"

using namespace panga;

Chromosome::Chromosome(const Genome* genome) :
    BitVector(genome->BitsRequired()),
    _genome(genome) {
}

Chromosome::~Chromosome() {
}

const Genome* Chromosome::GetGenome() const {
    return this->_genome;
}

void Chromosome::Randomize(RandomWrapper* randomWrapper) {
    for (size_t i = 0; i < this->_bytesCount; i++) {
        this->_bytes[i] = randomWrapper->RandomByte();
    }
}

bool Chromosome::DecodeBooleanGene(size_t geneIndex) const {
    assert(geneIndex < this->_genome->GetGeneCount());
    assert(geneIndex >= this->_genome->GetFirstBooleanGeneIndex());

    size_t bitIndex = (geneIndex - this->_genome->GetFirstBooleanGeneIndex()) + this->_genome->GetFirstBooleanGeneBitIndex();
    return this->Get(bitIndex);
}

void Chromosome::EncodeBooleanGene(size_t geneIndex, bool value) {
    assert(geneIndex < this->_genome->GetGeneCount());
    assert(geneIndex >= this->_genome->GetFirstBooleanGeneIndex());

    size_t bitIndex = (geneIndex - this->_genome->GetFirstBooleanGeneIndex()) + this->_genome->GetFirstBooleanGeneBitIndex();
    if (value) {
        this->Set(bitIndex);
    } else {
        this->Unset(bitIndex);
    }
}

std::byte* Chromosome::GetRawGene(size_t geneIndex, size_t* geneBitWidth) const {
    assert(geneIndex < this->_genome->GetFirstBooleanGeneIndex());
    assert(geneBitWidth != nullptr);

    size_t geneStartBitIndex = this->_genome->GetGeneStartBitIndex(geneIndex);
    *geneBitWidth = this->_genome->GetGeneBitWitdh(geneIndex);

    // Require raw genes to be byte-aligned.
    assert(geneStartBitIndex % 8 == 0);
    assert(*geneBitWidth % 8 == 0);

    return this->_bytes + (geneStartBitIndex / 8);
}

void Chromosome::FlipMutator(Chromosome* chromosome, double mutationPercentage, RandomWrapper* randomWrapper) {
    // Calculate the number of bits we should flip as the mutation percentage times the length of the chromosome (in bits)
    size_t chromosomeLength = chromosome->GetBitCount();
    // TODO: casts
    unsigned int numBitsToFlip = (unsigned int)round((double)chromosomeLength * mutationPercentage);

    // Flip the number of bits we calculated by choosing a random index and flipping it
    // Note: It's possible the same index could be flipped multiple times
    for (unsigned int i = 0; i < numBitsToFlip; i++) {
        size_t index = randomWrapper->RandomInteger<size_t>(0, chromosomeLength - 1);

        chromosome->Flip(index);
    }
}
