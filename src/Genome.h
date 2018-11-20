#pragma once

#include <vector>

namespace panga {

/**
 * Genome class provides a description of the range of possible values which an Individual could have.
 */
class Genome {
protected:
    std::vector<size_t> _geneStartBitIndex;
    std::vector<size_t> _geneBitWidth;
    size_t _firstBooleanGeneBitIndex;
    size_t _booleanGeneCount;

public:
    Genome();
    ~Genome();

    /**
     * @return The starting bit index of the newly-added gene.
     */
    template <bool ByteAlign = false>
    size_t AddGene(size_t geneBitWidth) {
        assert(geneBitWidth != 0);
        assert(this->_geneStartBitIndex.size() == this->_geneBitWidth.size());

        // First gene starts at bit index 0.
        // Subsequent genes start at the previous index + the previous bit width.
        size_t geneBitStartIndex = this->_geneStartBitIndex.empty() ?
            0 :
            this->_geneStartBitIndex.back() + this->_geneBitWidth.back();

        // Align the bits underlying the gene such that it begins and ends at a byte border.
        if (ByteAlign) {
            size_t bitGap = geneBitWidth % 8;
            if (bitGap != 0) {
                geneBitWidth += 8 - bitGap;
            }
            bitGap = geneBitStartIndex % 8;
            if (bitGap != 0) {
                geneBitStartIndex += 8 - bitGap;
            }
        }

        this->_geneStartBitIndex.push_back(geneBitStartIndex);
        this->_geneBitWidth.push_back(geneBitWidth);

        // Boolean genes are always at the end of the chromosome.
        // Update the bit index for the start of the boolean genes.
        this->_firstBooleanGeneBitIndex = geneBitStartIndex + geneBitWidth;

        return geneBitStartIndex;
    }

    void SetBooleanGeneCount(size_t booleanGeneCount);
    void AddBooleanGenes(size_t count);

    /**
     * Get the number of boolean Genes in the Genome.
     */
    size_t GetBooleanGeneCount() const;

    /**
     * Get the bit offset where a gene starts.
     */
    size_t GetGeneStartBitIndex(size_t geneIndex) const;

    /**
     * Get the bit width for a gene.
     */
    size_t GetGeneBitWitdh(size_t geneIndex) const;
    size_t GetFirstBooleanGeneBitIndex() const;
    size_t GetFirstBooleanGeneIndex() const;
    size_t GetGeneCount() const;

    /**
     * Get the number of bits needed to encode this Genome.
     */
    size_t BitsRequired() const;
};

} // namespace panga
