//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cassert>

#include "BitVector.h"
#include "Genome.h"
#include "RandomWrapper.h"

namespace panga {

/**
 * The binary data acting as an instance of a Genome.<br/>
 * Length of Chromosome in bits will be the same as the bits required to
 * encode the Genome.<br/>
 * With the Genome, we know where all the genes are located in our binary data
 * and we can interpret the bits making up each gene.
 * @see Genome
 */
class Chromosome : public BitVector {
protected:
    const Genome* _genome;

public:
    Chromosome(const Genome* genome);
    ~Chromosome();

    const Genome* GetGenome() const;

    /**
     * Randomize all bits in the Chromosome.
     */
    void Randomize(RandomWrapper* randomWrapper);

    /**
     * Fetch the binary data representing a gene and interpret it as an integer
     * scaled between min and max.<br/>
     * If useGrayEncoding is true, we will treat the gene data as a gray-coded
     * number and decode it into a binary representation before scaling.
     * @param geneIndex The index of the gene we should interpret.
     * @param min The min value which the integer could assume.
     * @param max The max value which the integer could assume.
     * @return Integer representation of the gene in this Chromosome at
     * geneIndex.
     */
    template <typename IntegerType = uint64_t, bool useGrayEncoding = true>
    IntegerType DecodeIntegerGene(size_t geneIndex, IntegerType min = 0, IntegerType max = std::numeric_limits<IntegerType>::max()) const {
        if (min == max) {
            return min;
        }

        size_t geneStartIndex = this->_genome->GetGeneStartBitIndex(geneIndex);
        size_t geneWidth = this->_genome->GetGeneBitWitdh(geneIndex);

        assert(sizeof(IntegerType) * 8 >= geneWidth);

        IntegerType value = this->GetInt<IntegerType>(geneStartIndex, geneWidth);
        if (useGrayEncoding) {
            value = DecodeGray<IntegerType>(value);
        }
        return (value % (max - min)) + min;
    }

    /**
     * Take an integer value and encode it into the binary data representing
     * a gene in the Chromosome.<br/>
     * If useGrayEncoding is true, we will encode the integer into a gray-coded
     * value before writing it into the gene data.
     * @param geneIndex The index of the gene we should write to.
     * @param value The integer value we want to write into the Chromosome.
     */
    template <typename IntegerType = uint64_t, bool useGrayEncoding = true>
    void EncodeIntegerGene(size_t geneIndex, IntegerType value) {
        size_t geneStartIndex = this->_genome->GetGeneStartBitIndex(geneIndex);
        size_t geneWidth = this->_genome->GetGeneBitWitdh(geneIndex);

        assert(sizeof(IntegerType) * 8 >= geneWidth);

        if (useGrayEncoding) {
            value = EncodeGray<IntegerType>(value);
        }
        this->SetInt<IntegerType>(value, geneStartIndex, geneWidth);
    }

    /**
     * Fetch the binary data representing a gene and interpret it as a floating
     * point number scaled between min and max.<br/>
     * If useGrayEncoding is true, we will treat the gene data as a gray-coded
     * number and decode it into a binary representation before scaling.
     * @param geneIndex The index of the gene we should interpret.
     * @param min The min value which the float could assume.
     * @param max The max value which the float could assume.
     * @return Floating point representation of the gene in this Chromosome at
     * geneIndex.
     */
    template <typename FloatType = double, bool useGrayEncoding = true>
    FloatType DecodeFloatGene(size_t geneIndex, FloatType min, FloatType max) const {
        uint64_t integerValue = this->DecodeIntegerGene<uint64_t, useGrayEncoding>(geneIndex);
        size_t geneWidth = this->_genome->GetGeneBitWitdh(geneIndex);
        return DecodeFloat<FloatType, uint64_t>(integerValue, geneWidth, min, max);
    }

    /**
     * Take a floating point value and encode it into the binary data
     * representing a gene in the Chromosome.<br/>
     * If useGrayEncoding is true, we will encode value into a gray-coded
     * value before writing it into the gene data.
     * @param geneIndex The index of the gene we should write to.
     * @param value The floating point value we want to write into the
     * Chromosome.
     */
    template <typename FloatType = double, bool useGrayEncoding = true>
    void EncodeFloatGene(size_t geneIndex, FloatType value, FloatType min, FloatType max) {
        size_t geneWidth = this->_genome->GetGeneBitWitdh(geneIndex);
        uint64_t integerValue = EncodeFloat<FloatType, uint64_t>(value, geneWidth, min, max);
        EncodeIntegerGene<uint64_t, useGrayEncoding>(geneIndex, integerValue);
    }

    /**
     * Return true if the gene at geneIndex is set and false if the gene is
     * not set.<br/>
     * Note: geneIndex must be the index of a boolean gene and not a gene
     * with a bit width - even if that bit width is 1.
     */
    bool DecodeBooleanGene(size_t geneIndex) const;

    /**
     * Set the bit value at geneIndex.<br/>
     * Note: geneIndex must be the index of a boolean gene and not a gene
     * with a bit width - even if that bit width is 1.
     * @param value If true, the bit at geneIndex is set. If false, the bit
     * at geneIndex is unset.
     */
    void EncodeBooleanGene(size_t geneIndex, bool value);

    /**
     * Get the raw binary data from the Chromosome where the gene at geneIndex
     * is located. Doesn't interpret the binary data.
     * @param geneBitWidth An out param which receives the bit width of the
     * gene at geneIndex.
     */
    std::byte* GetRawGene(size_t geneIndex, size_t* geneBitWidth) const;

    /**
     * Decode a binary integer from a gray-encoded value.
     * @param grayValue Gray-encoded value
     * @return Binary integer value
     */
    template <typename IntegerType = uint64_t>
    static IntegerType DecodeGray(IntegerType grayValue) {
        IntegerType binaryValue = grayValue;
        while (grayValue >>= 1) {
            binaryValue ^= grayValue;
        }
        return binaryValue;
    }

    /**
     * Encode a binary integer value into a gray representation.
     * @param binaryValue Binary integer value
     * @return Gray-encoded value
     */
    template <typename IntegerType = uint64_t>
    static IntegerType EncodeGray(IntegerType binaryValue) {
        return binaryValue ^ (binaryValue >> 1);
    }

    /**
     * Decode a floating point number from an integer representation.
     * @param integerValue The integer representation we will decode.
     * @param bitWidth How many bits of integerValue do we want to decode.
     * @param min Min value for the float.
     * @param max Max value for the float.
     * @see EncodeFloat
     */
    template <typename FloatType = double, typename IntegerType = uint64_t>
    static FloatType DecodeFloat(IntegerType integerValue, size_t bitWidth, FloatType min, FloatType max) {
        auto localMax = std::numeric_limits<IntegerType>::max() >> (sizeof(IntegerType) * 8 - bitWidth);
        FloatType factor = (FloatType)integerValue / (FloatType)localMax;
        return factor * (max - min) + min;
    }

    /**
     * Encode a floating point number into an integer representation.
     * @param floatVal Float value to encode.
     * @param bitWidth How many bits should we limit the integer representation
     * to.
     * @param min Min value for the float.
     * @param max Max value for the float.
     * @see DecodeFloat
     */
    template <typename FloatType = double, typename IntegerType = uint64_t>
    static IntegerType EncodeFloat(FloatType floatVal, size_t bitWidth, FloatType min, FloatType max) {
        auto localMax = std::numeric_limits<IntegerType>::max() >> (sizeof(IntegerType) * 8 - bitWidth);
        FloatType factor = (floatVal - min) / (max - min);
        return factor > 1.0 ?
            localMax :
            (IntegerType)(factor * localMax);
    }

    template <bool ignoreGeneBoundaries = true>
    static void UniformCrossover(Chromosome* parent1, Chromosome* parent2, Chromosome* offspring, RandomWrapper* randomWrapper) {
        assert(parent1->_bitCount == parent2->_bitCount);

        if (ignoreGeneBoundaries) {
            for (size_t i = 0; i < parent1->_bytesCount; i++) {
                std::byte mask = randomWrapper->RandomByte();
                offspring->_bytes[i] = (mask & parent1->_bytes[i]) | (~mask & parent2->_bytes[i]);
            }
        } else {
            const Genome* genome = parent1->GetGenome();
            for (size_t i = 0; i < genome->GetGeneCount(); i++) {
                Chromosome* chunkSource = randomWrapper->CoinFlip(0.5) ? parent1 : parent2;
                size_t geneStartBitIndex = genome->GetGeneStartBitIndex(i);
                size_t geneBitWidth = genome->GetGeneBitWitdh(i);
                if (i >= genome->GetFirstBooleanGeneIndex()) {
                    if (chunkSource->Get(geneStartBitIndex)) {
                        offspring->Set(geneStartBitIndex);
                    } else {
                        offspring->Unset(geneStartBitIndex);
                    }
                } else {
                    chunkSource->SubVector(offspring, geneStartBitIndex, geneStartBitIndex, geneBitWidth);
                }
            }
        }
    }

    template <bool ignoreGeneBoundaries = true>
    static void KPointCrossover(size_t k, Chromosome* parent1, Chromosome* parent2, Chromosome* offspring, RandomWrapper* randomWrapper) {
        size_t left = 0;
        size_t totalBits = ignoreGeneBoundaries ?
            parent1->GetBitCount() :
            parent1->_genome->GetGeneCount();

        for (size_t i = 0; i < k + 1; i++) {
            // Copy a chunk of bits starting with left and ending at right.
            size_t right = randomWrapper->RandomInteger(left, totalBits);

            // If this is the last chunk to take, make sure we take all the rest of the bits.
            if (i == k) {
                right = totalBits;
            }

            // Decide if this chunk of bits will come from parent1 or parent2 (this alternates with each step).
            Chromosome* chunkSource = i % 2 == 0 ? parent1 : parent2;

            // Copy the current chunk.
            if (left < totalBits) {
                size_t startIndex = left;
                size_t endIndex = right;

                if (!ignoreGeneBoundaries) {
                    startIndex = parent1->_genome->GetGeneStartBitIndex(left);
                    endIndex = parent1->_genome->GetGeneStartBitIndex(right) + parent1->_genome->GetGeneBitWitdh(right);
                }

                chunkSource->SubVector(offspring, startIndex, startIndex, endIndex - startIndex);
            }

            left = right;
        }
    }

    static void FlipMutator(Chromosome* chromosome, double mutationPercentage, RandomWrapper* randomWrapper);

protected:
    Chromosome(const Chromosome&);
};

} // namespace panga
