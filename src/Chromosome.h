//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef CHROMOSOME_H__
#define CHROMOSOME_H__

#include <cassert>
#include <cstdint>

#include "BitVector.h"
#include "Genome.h"

namespace panga {

class RandomWrapper;

/**
 * The binary data acting as an instance of a Genome.<br/>
 * Length of Chromosome in bits will be the same as the bits required to
 * encode the Genome.<br/>
 * With the Genome, we know where all the genes are located in our binary data
 * and we can interpret the bits making up each gene.
 * @see Genome
 */
class Chromosome : public BitVector {
public:
    explicit Chromosome(const Genome& genome);
    Chromosome(const Chromosome& rhs) = delete;
    Chromosome(Chromosome&& rhs) = default;
    Chromosome& operator=(const Chromosome& rhs) = delete;
    ~Chromosome() = default;

    /**
     * Get a read-only reference to the Genome used by this Chromosome.
     */
    const Genome& GetGenome() const;

    /**
     * Randomize all bits in the Chromosome.
     */
    void Randomize(RandomWrapper* random);

    /**
     * Fetch the binary data representing a gene and interpret it as an integer
     * scaled between min and max.<br/>
     * If use_gray_encoding is true, we will treat the gene data as a gray-coded
     * number and decode it into a binary representation before scaling.
     * @param gene_index The index of the gene we should interpret.
     * @param min The min value which the integer could assume.
     * @param max The max value which the integer could assume.
     * @return Integer representation of the gene in this Chromosome at |gene_index|.
     */
    template <typename IntegerType = uint64_t, bool use_gray_encoding = true>
    IntegerType DecodeIntegerGene(size_t gene_index, IntegerType min = 0, IntegerType max = std::numeric_limits<IntegerType>::max()) const {
        if (min == max) {
            return min;
        }

        const size_t gene_bit_index = genome_.GetGeneStartBitIndex(gene_index);
        const size_t gene_width = genome_.GetGeneBitWitdh(gene_index);
        assert(sizeof(IntegerType) * CHAR_BIT >= gene_width);

        auto value = GetInt<IntegerType>(gene_bit_index, gene_width);
        if (use_gray_encoding) {
            value = DecodeGray<IntegerType>(value);
        }

        // Clamp into range.
        return (value % (max - min)) + min;
    }

    /**
     * Take an integer value and encode it into the binary data representing
     * a gene in the Chromosome.<br/>
     * If use_gray_encoding is true, we will encode the integer into a gray-coded
     * value before writing it into the gene data.
     * @param gene_index The index of the gene we should write to.
     * @param value The integer value we want to write into the Chromosome.
     */
    template <typename IntegerType = uint64_t, bool use_gray_encoding = true>
    void EncodeIntegerGene(size_t gene_index, IntegerType value) {
        const size_t gene_bit_index = genome_.GetGeneStartBitIndex(gene_index);
        const size_t gene_width = genome_.GetGeneBitWitdh(gene_index);
        assert(sizeof(IntegerType) * CHAR_BIT >= gene_width);

        if (use_gray_encoding) {
            value = EncodeGray<IntegerType>(value);
        }
        SetInt<IntegerType>(value, gene_bit_index, gene_width);
    }

    /**
     * Fetch the binary data representing a gene and interpret it as a floating
     * point number scaled between min and max.<br/>
     * If use_gray_encoding is true, we will treat the gene data as a gray-coded
     * number and decode it into a binary representation before scaling.
     * @param gene_index The index of the gene we should interpret.
     * @param min The min value which the float could assume.
     * @param max The max value which the float could assume.
     * @return Floating point representation of the gene in this Chromosome at
     * gene_index.
     */
    template <typename FloatType = double, bool use_gray_encoding = true>
    FloatType DecodeFloatGene(size_t gene_index, FloatType min, FloatType max) const {
        const uint64_t int_value = DecodeIntegerGene<uint64_t, use_gray_encoding>(gene_index);
        const size_t gene_width = genome_.GetGeneBitWitdh(gene_index);
        return DecodeFloat<FloatType, uint64_t>(int_value, gene_width, min, max);
    }

    /**
     * Take a floating point value and encode it into the binary data
     * representing a gene in the Chromosome.<br/>
     * If use_gray_encoding is true, we will encode value into a gray-coded
     * value before writing it into the gene data.
     * @param gene_index The index of the gene we should write to.
     * @param value The floating point value we want to write into the
     * Chromosome.
     */
    template <typename FloatType = double, bool use_gray_encoding = true>
    void EncodeFloatGene(size_t gene_index, FloatType value, FloatType min, FloatType max) {
        const size_t gene_width = genome_.GetGeneBitWitdh(gene_index);
        const uint64_t int_value = EncodeFloat<FloatType, uint64_t>(value, gene_width, min, max);
        EncodeIntegerGene<uint64_t, use_gray_encoding>(gene_index, int_value);
    }

    /**
     * Return true if the gene at |gene_index| is set and false if the gene is
     * not set.<br/>
     * Note: |gene_index| must be the index of a boolean gene and not a gene
     * with a bit width - even if that bit width is 1.
     */
    bool DecodeBooleanGene(size_t gene_index) const;

    /**
     * Set the bit value at |gene_index|.<br/>
     * Note: |gene_index| must be the index of a boolean gene and not a gene
     * with a bit width - even if that bit width is 1.
     * @param value If true, the bit at |gene_index| is set. If false, the bit
     * at |gene_index| is unset.
     */
    void EncodeBooleanGene(size_t gene_index, bool value);

    /**
     * Get the raw binary data from the Chromosome where the gene at |gene_index|
     * is located. Doesn't interpret the binary data.
     * @param gene_bit_width An out param which receives the bit width of the
     * gene at |gene_index|.
     */
    std::byte* GetRawGene(size_t gene_index, size_t* gene_bit_width);

    /**
     * Decode a binary integer from a gray-encoded value.
     * @param gray_value Gray-encoded value
     * @return Binary integer value
     */
    template <typename IntegerType = uint64_t>
    static IntegerType DecodeGray(IntegerType gray_value) {
        IntegerType binary_value = gray_value;
        while (gray_value >>= 1) {
            binary_value ^= gray_value;
        }
        return binary_value;
    }

    /**
     * Encode a binary integer value into a gray representation.
     * @param binary_value Binary integer value
     * @return Gray-encoded value
     */
    template <typename IntegerType = uint64_t>
    static IntegerType EncodeGray(IntegerType binary_value) {
        return binary_value ^ (binary_value >> 1);
    }

    /**
     * Decode a floating point number from an integer representation.
     * @param int_value The integer representation we will decode.
     * @param bit_width How many bits of int_value do we want to decode.
     * @param min Min value for the float.
     * @param max Max value for the float.
     * @see EncodeFloat
     */
    template <typename FloatType = double, typename IntegerType = uint64_t>
    static FloatType DecodeFloat(IntegerType int_value, size_t bit_width, FloatType min, FloatType max) {
        constexpr size_t bits_per_byte = 8;
        const IntegerType local_max = std::numeric_limits<IntegerType>::max() >> (sizeof(IntegerType) * bits_per_byte - bit_width);
        const FloatType factor = static_cast<FloatType>(int_value) / static_cast<FloatType>(local_max);
        return factor * (max - min) + min;
    }

    /**
     * Encode a floating point number into an integer representation.
     * @param float_value Float value to encode.
     * @param bit_width How many bits should we limit the integer representation
     * to.
     * @param min Min value for the float.
     * @param max Max value for the float.
     * @see DecodeFloat
     */
    template <typename FloatType = double, typename IntegerType = uint64_t>
    static IntegerType EncodeFloat(FloatType float_value, size_t bit_width, FloatType min, FloatType max) {
        constexpr size_t bits_per_byte = 8;
        const IntegerType local_max = std::numeric_limits<IntegerType>::max() >> (sizeof(IntegerType) * bits_per_byte - bit_width);
        const FloatType factor = (float_value - min) / (max - min);
        return factor > 1.0 ?
            local_max :
            static_cast<IntegerType>(factor * local_max);
    }

    /**
     * Perform uniform crossover between |parent1| and |parent2| and store the result in |offspring|.
     * @param ignore_gene_boundaries If false, gene boundaries will be respected. Instead of some bits from a gene being copied from |parent1| and some from |parent2|, all of the bits containing the gene will be copied together from a single parent. Each gene still has uniformly equal chance of being copied from either parent.<br/>If true, gene boundaries are ignored and every bit in the chromosome has an equal chance to be copied from either parent.
     */
    static void UniformCrossover(const Chromosome& parent1, const Chromosome& parent2, Chromosome* offspring, RandomWrapper* random, bool ignore_gene_boundaries = true);

    /**
     * Perform k-point crossover between |parent1| and |parent2| and store the result in |offspring|.
     * @param ignore_gene_boundaries If false, gene boundaries will be respected. Instead of some bits from a gene being copied from |parent1| and some from |parent2|, all of the bits containing the gene will be copied together from a single parent. Genes are copied in chunks according to the cut points.<br/>If true, gene boundaries are ignored and the chromosome is treated as a single chunk of bits which is split into chunks according to the cut points.
     */
    static void KPointCrossover(size_t k, const Chromosome& parent1, const Chromosome& parent2, Chromosome* offspring, RandomWrapper* random, bool ignore_gene_boundaries = true);

    /**
     * Perform flip mutation on |chromosome|.<br/>
     * Every bit in |chromosome| will have |mutation_percentage| chance of flipping.
     */
    static void FlipMutator(Chromosome* chromosome, double mutation_percentage, RandomWrapper* random);

private:
    const Genome& genome_;
};

}  // namespace panga

#endif  // CHROMOSOME_H__
