//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>

namespace panga {

/**
 * Provides a representation of the genes which, taken together, represent a
 * genome for members of a species.<br/>
 * Using the Genome, we may decode a block of binary data and extract the bits
 * representing each gene from the data. (This is what Chromosome does).<br/>
 * The Genome data may be constructed by adding genes one-at-a-time via AddGene
 * along with an optional number of boolean genes.<br/>
 * The genome is structured into two pieces:<br/>
 *    1. Genes with a bit width<br/>
 *    2. Boolean genes which have only one bit value<br/>
 * For performance reasons, all of the boolean genes are located at the end of
 * the Genome. It is preferable to add a boolean gene than add a gene with bit
 * width of 1.
 * @see AddGene
 * @see Chromosome
 */
class Genome {
protected:
    struct Gene {
        size_t startBitIndex;
        size_t bitWidth;
    };

    std::vector<Gene> _genes;
    size_t _firstBooleanGeneBitIndex;
    size_t _booleanGeneCount;

public:
    Genome();
    ~Genome();

    /**
     * Add a gene to the Genome.<br/>
     * The new gene will be added to the end of the Genome.<br/>
     * If byteAlign is true, we will shift the starting offset and bit width
     * such that this gene begins and ends on a byte-boundary.<br/>
     * This can be very useful if you want to access the memory backing this
     * gene later directly (via something like Chromosome::GetRawGene).
     * @param geneBitWidth The width of the gene we want to add.
     * @see Chromosome::GetRawGene
     * @return The gene index of the newly-added gene.
     */
    template <bool byteAlign = false>
    size_t AddGene(size_t geneBitWidth) {
        assert(geneBitWidth != 0);

        // First gene starts at bit index 0.
        // Subsequent genes start at the previous index + the previous bit width.
        size_t geneBitStartIndex = 0;
        if (!this->_genes.empty()) {
            const Gene& gene = this->_genes.back();
            geneBitStartIndex = gene.startBitIndex + gene.bitWidth;
        }

        // Align the bits underlying the gene such that it begins and ends at a byte border.
        if (byteAlign) {
            size_t bitGap = geneBitWidth % 8;
            if (bitGap != 0) {
                geneBitWidth += 8 - bitGap;
            }
            bitGap = geneBitStartIndex % 8;
            if (bitGap != 0) {
                geneBitStartIndex += 8 - bitGap;
            }
        }

        size_t geneIndex = this->_genes.size();
        this->_genes.push_back({geneBitStartIndex,geneBitWidth});

        // Boolean genes are always at the end of the chromosome.
        // Update the bit index for the start of the boolean genes.
        this->_firstBooleanGeneBitIndex = geneBitStartIndex + geneBitWidth;

        return geneIndex;
    }

    /**
     * Set the number of boolean genes in this Genome.<br/>
     * This overwrites any value currently in the boolean gene count.
     */
    void SetBooleanGeneCount(size_t booleanGeneCount);

    /**
     * Add count boolean genes to the boolean gene count for this Genome.<br/>
     * Does not return the gene indices because adding more non-boolean genes
     * to the Genome can change the boolean gene indices.
     * @see GetFirstBooleanGeneIndex
     * @see GetFirstBooleanGeneBitIndex
     */
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
     * @param geneIndex Index of the gene we should return the width for.
     * @return The width in bits of gene at index geneIndex.
     */
    size_t GetGeneBitWitdh(size_t geneIndex) const;

    /**
     * Get the bit index of the first boolean gene in the Genome.
     */
    size_t GetFirstBooleanGeneBitIndex() const;

    /**
     * Get the gene index of the first boolean gene in the Genome.
     */
    size_t GetFirstBooleanGeneIndex() const;

    /**
     * Get the count of genes in the Genome.<br/>
     * Note: Includes the count of boolean genes.
     */
    size_t GetGeneCount() const;

    /**
     * Get the number of bits needed to encode this Genome.
     */
    size_t BitsRequired() const;
};

} // namespace panga
