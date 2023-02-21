//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef GENOME_H__
#define GENOME_H__

#include <cassert>
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
        size_t start_bit_index_;
        size_t bit_width_;
    };

public:
    Genome() = default;
    Genome(const Genome& rhs) = delete;
    Genome& operator=(const Genome& rhs) = delete;
    ~Genome() = default;

    /**
     * Add a gene to the Genome.<br/>
     * The new gene will be added to the end of the Genome.<br/>
     * This can be very useful if you want to access the memory backing this
     * gene later directly (via something like Chromosome::GetRawGene).
     * @param bit_width The width of the gene we want to add.
     * @param byte_align If true, we will shift the starting offset and bit width such that this gene begins and ends on a byte-boundary.
     * @see Chromosome::GetRawGene
     * @return The gene index of the newly-added gene.
     */
    size_t AddGene(size_t bit_width, bool byte_align = false);

    /**
     * Set the number of boolean genes in this Genome.<br/>
     * This overwrites any value currently in the boolean gene count.
     */
    void SetBooleanGeneCount(size_t boolean_gene_count);

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
    size_t GetGeneStartBitIndex(size_t gene_index) const;

    /**
     * Get the bit width for a gene.
     * @param gene_index Index of the gene we should return the width for.
     * @return The width in bits of gene at index gene_index.
     */
    size_t GetGeneBitWitdh(size_t gene_index) const;

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

private:
    std::vector<Gene> genes_;
    size_t first_boolean_gene_bit_index_;
    size_t boolean_gene_count_;
};

}  // namespace panga

#endif  // GENOME_H__
