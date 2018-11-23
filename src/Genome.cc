//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>

#include "Genome.h"

using namespace panga;

Genome::Genome() :
    _firstBooleanGeneBitIndex(0),
    _booleanGeneCount(0) {
}

Genome::~Genome() {
}

void Genome::SetBooleanGeneCount(size_t booleanGeneCount) {
    this->_booleanGeneCount = booleanGeneCount;
}

void Genome::AddBooleanGenes(size_t count) {
    this->_booleanGeneCount += count;
}

size_t Genome::GetFirstBooleanGeneBitIndex() const {
    return this->_firstBooleanGeneBitIndex;
}

size_t Genome::GetFirstBooleanGeneIndex() const {
    return this->_genes.size();
}

size_t Genome::GetGeneCount() const {
    return this->_genes.size() + this->_booleanGeneCount;
}

size_t Genome::GetGeneStartBitIndex(size_t geneIndex) const {
    assert(geneIndex < this->GetGeneCount());

    return geneIndex >= this->_genes.size() ?
        this->_firstBooleanGeneBitIndex + this->_genes.size() - geneIndex
        : this->_genes[geneIndex].startBitIndex;
}

size_t Genome::GetGeneBitWitdh(size_t geneIndex) const {
    assert(geneIndex < this->GetGeneCount());

    return geneIndex >= this->_genes.size() ?
        1
        : this->_genes[geneIndex].bitWidth;
}

size_t Genome::GetBooleanGeneCount() const {
    return this->_booleanGeneCount;
}

size_t Genome::BitsRequired() const {
    if (this->_genes.empty()) {
        return this->_booleanGeneCount;
    }

    const Gene& gene = this->_genes.back();
    return gene.startBitIndex + gene.bitWidth + this->_booleanGeneCount;
}
