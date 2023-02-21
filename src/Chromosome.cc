//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <cassert>

#include "Chromosome.h"
#include "Genome.h"
#include "RandomWrapper.h"

namespace {

constexpr size_t BitsPerByte = 8U;

}  // namespace

namespace panga {

Chromosome::Chromosome(const Genome& genome) :
    BitVector(genome.BitsRequired()),
    genome_(genome) {
}

const Genome& Chromosome::GetGenome() const {
    return genome_;
}

void Chromosome::Randomize(RandomWrapper* random) {
    for (auto& byte : GetBytesWritable()) {
        byte = random->RandomByte();
    }
}

bool Chromosome::DecodeBooleanGene(size_t gene_index) const {
    assert(gene_index < genome_.GetGeneCount());
    assert(gene_index >= genome_.GetFirstBooleanGeneIndex());

    const size_t bit_index = (gene_index - genome_.GetFirstBooleanGeneIndex()) + genome_.GetFirstBooleanGeneBitIndex();
    return Get(bit_index);
}

void Chromosome::EncodeBooleanGene(size_t gene_index, bool value) {
    assert(gene_index < genome_.GetGeneCount());
    assert(gene_index >= genome_.GetFirstBooleanGeneIndex());

    const size_t bit_index = (gene_index - genome_.GetFirstBooleanGeneIndex()) + genome_.GetFirstBooleanGeneBitIndex();
    if (value) {
        Set(bit_index);
    } else {
        Unset(bit_index);
    }
}

std::byte* Chromosome::GetRawGene(size_t gene_index, size_t* gene_bit_width) {
    assert(gene_index < genome_.GetFirstBooleanGeneIndex());
    assert(gene_bit_width != nullptr);

    const size_t gene_bit_index = genome_.GetGeneStartBitIndex(gene_index);
    *gene_bit_width = genome_.GetGeneBitWitdh(gene_index);

    // Require raw genes to be byte-aligned.
    assert(gene_bit_index % BitsPerByte == 0);
    assert(*gene_bit_width % BitsPerByte == 0);

    return GetBytesWritable().data() + (gene_bit_index / BitsPerByte);
}

// static
void Chromosome::UniformCrossover(const Chromosome& parent1, const Chromosome& parent2, Chromosome* offspring, RandomWrapper* random, bool ignore_gene_boundaries) {
    assert(parent1.GetBitCount() == parent2.GetBitCount());

    // Make sure |offspring| has enough storage for these bits.
    offspring->Resize(parent1.GetBitCount());

    if (ignore_gene_boundaries) {
        // When we are ignoring the gene boundaries, every bit has equal random chance to be copied from either parent1 or parent2.
        const auto& parent1_bytes = parent1.GetBytes();
        const auto& parent2_bytes = parent2.GetBytes();
        auto& offspring_bytes = offspring->GetBytesWritable();

        // It's possible that the byte buffers backing each parent may have allocated a different number of bytes. We should be safe to use the number of bytes necessary to store the bit count because they both report to contain the same number of bits.
        const size_t byte_count = offspring_bytes.size();
        for (size_t i = 0; i < byte_count; i++) {
            // Use a random byte to determine splitting.
            // Bits turned on in this byte will be pulled from parent1 and bits turned off will be pulled from parent2.
            const std::byte mask = random->RandomByte();
            offspring_bytes[i] = (mask & parent1_bytes[i]) | (~mask & parent2_bytes[i]);
        }
    } else {
        // We need to respect the gene boundaries, which effectively means each gene has equal random chance to be copied wholesale from either parent1 or parent2.
        const Genome& genome = parent1.GetGenome();
        for (size_t i = 0; i < genome.GetGeneCount(); i++) {
            constexpr double equal_chance = 0.5;
            const Chromosome& chunk_source = random->CoinFlip(equal_chance) ? parent1 : parent2;
            const size_t gene_bit_index = genome.GetGeneStartBitIndex(i);
            const size_t gene_bit_width = genome.GetGeneBitWitdh(i);

            // All boolean genes are located at the end of the vector.
            if (i < genome.GetFirstBooleanGeneIndex()) {
                // Copy all bits necessary for the gene out of the parent.
                chunk_source.SubVector(offspring, gene_bit_index, gene_bit_index, gene_bit_width);
            } else {
                // TODO(boingoing): Probably makes sense to only copy the non-boolean genes via this loop and separately copy all the boolean genes similar to how we do uniform crossover when ignoring gene boundaries (ie: copying bytes at a time instead of bits).
                if (chunk_source.Get(gene_bit_index)) {
                    offspring->Set(gene_bit_index);
                } else {
                    offspring->Unset(gene_bit_index);
                }
            }
        }
    }
}

// static
void Chromosome::KPointCrossover(size_t k, const Chromosome& parent1, const Chromosome& parent2, Chromosome* offspring, RandomWrapper* random, bool ignore_gene_boundaries) {
    assert(parent1.GetBitCount() == parent2.GetBitCount());

    const size_t bit_count = parent1.GetBitCount();
    // Make sure |offspring| has enough storage for these bits.
    offspring->Resize(bit_count);

    if (ignore_gene_boundaries) {
        // When we are ignoring the gene boundaries, cut points can happen anywhere in the chromosome.
        size_t left_bit_index = 0;

        // We need to copy k+1 chunks from the parents.
        for (size_t i = 0; i < k + 1U; i++) {
            // Copy a chunk of bits starting with left and ending at a randomly-chosen right.
            // If this is the last chunk to take, make sure we take all the rest of the bits (ie: set right to bit count).
            const size_t right_bit_index = (i == k) ? bit_count : random->RandomInteger(left_bit_index, bit_count);

            // Decide if this chunk of bits will come from parent1 or parent2 (this alternates with each chunk).
            const Chromosome& chunkSource = i % 2U == 0 ? parent1 : parent2;

            // It's possible that one of the randomly chosen cut points was at the end which means there's nothing left for us to copy now.
            if (left_bit_index < bit_count) {
                // Copy the current chunk of (right_bit_index - left_bit_index) bits beginning at left_bit_index and ending at right_bit_index.
                chunkSource.SubVector(offspring, left_bit_index, left_bit_index, right_bit_index - left_bit_index);
            }

            // Adjust left so that the next chunk will start where right ended.
            left_bit_index = right_bit_index;
        }
    } else {
        // We need to respect the gene boundaries, cut points can only happen at gene boundaries.
        const Genome& genome = parent1.GetGenome();
        const size_t gene_count = genome.GetGeneCount();
        size_t left_gene_index = 0;

        // We need to copy genes in k+1 chunks from the parents.
        for (size_t i = 0; i < k + 1U; i++) {
            // Copy a chunk of genes starting with left and ending at a randomly-chosen right.
            // If this is the last chunk to take, make sure we take all the rest of the genes (ie: set right to gene count).
            const size_t right_gene_index = (i == k) ? gene_count : random->RandomInteger(left_gene_index, gene_count);

            // Decide if this chunk of genes will come from parent1 or parent2 (this alternates with each step).
            const Chromosome& chunkSource = i % 2U == 0 ? parent1 : parent2;

            // It's possible that one of the randomly chosen cut points was at the end which means there's nothing left for us to copy now.
            if (left_gene_index < gene_count) {
                // The current chunk of bits we need to copy from the parent starts at the bit index of left gene and ends at the ending bit index of right gene.
                const size_t start_bit_index = genome.GetGeneStartBitIndex(left_gene_index);
                const size_t end_bit_index = genome.GetGeneStartBitIndex(right_gene_index) + genome.GetGeneBitWitdh(right_gene_index);
                chunkSource.SubVector(offspring, start_bit_index, start_bit_index, end_bit_index - start_bit_index);
            }

            left_gene_index = right_gene_index;
        }
    }
}

// static
void Chromosome::FlipMutator(Chromosome* chromosome, double mutation_percentage, RandomWrapper* random) {
    // Calculate the number of bits we should flip as the mutation percentage times the length of the chromosome (in bits).
    const size_t bit_count = chromosome->GetBitCount();
    const size_t bits_to_flip = std::llround(bit_count * mutation_percentage);

    // Flip the number of bits we calculated by choosing a random index and flipping it.
    // Note: It's possible the same index could be flipped multiple times.
    for (size_t i = 0; i < bits_to_flip; i++) {
        const auto index = random->RandomInteger<size_t>(0, bit_count - 1U);
        chromosome->Flip(index);
    }
}

}  // namespace panga
