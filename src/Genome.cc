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
    assert(this->_geneStartBitIndex.size() == this->_geneBitWidth.size());
    
    return this->_geneStartBitIndex.size();
}

size_t Genome::GetGeneCount() const {
    assert(this->_geneStartBitIndex.size() == this->_geneBitWidth.size());

    return this->_geneStartBitIndex.size() + this->_booleanGeneCount;
}

size_t Genome::GetGeneStartBitIndex(size_t geneIndex) const {
    assert(geneIndex < this->GetGeneCount());

    return geneIndex >= this->_geneStartBitIndex.size() ?
        this->_firstBooleanGeneBitIndex + this->_geneStartBitIndex.size() - geneIndex
        : this->_geneStartBitIndex[geneIndex];
}

size_t Genome::GetGeneBitWitdh(size_t geneIndex) const {
    assert(geneIndex < this->GetGeneCount());

    return geneIndex >= this->_geneBitWidth.size() ?
        1
        : this->_geneBitWidth[geneIndex];
}

size_t Genome::GetBooleanGeneCount() const {
    return this->_booleanGeneCount;
}

size_t Genome::BitsRequired() const {
    assert(this->_geneStartBitIndex.size() == this->_geneBitWidth.size());

    if (this->_geneStartBitIndex.empty()) {
        return this->_booleanGeneCount;
    }

    return this->_geneStartBitIndex.back() + this->_geneBitWidth.back() + this->_booleanGeneCount;
}
