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

class Chromosome : public BitVector {
protected:
    const Genome* _genome;

public:
    Chromosome(const Genome* genome);
    ~Chromosome();

    const Genome* GetGenome();

    /**
     * Randomize all bits in the Chromosome.
     */
    void Randomize(RandomWrapper* randomWrapper);

    template <typename IntegerType = uint64_t, bool useGrayEncoding = true>
    IntegerType DecodeIntegerGene(size_t geneIndex, IntegerType min = 0, IntegerType max = std::numeric_limits<IntegerType>::max()) {
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

    template <typename FloatType = double, bool useGrayEncoding = true>
    FloatType DecodeFloatGene(size_t geneIndex, FloatType min, FloatType max) {
        uint64_t integerValue = this->DecodeIntegerGene<uint64_t, useGrayEncoding>(geneIndex);
        size_t geneWidth = this->_genome->GetGeneBitWitdh(geneIndex);
        return DecodeFloat<FloatType, uint64_t>(integerValue, geneWidth, min, max);
    }

    template <typename FloatType = double, bool useGrayEncoding = true>
    void EncodeFloatGene(size_t geneIndex, FloatType value, FloatType min, FloatType max) {
        size_t geneWidth = this->_genome->GetGeneBitWitdh(geneIndex);
        uint64_t integerValue = EncodeFloat<FloatType, uint64_t>(value, geneWidth, min, max);
        EncodeIntegerGene<uint64_t, useGrayEncoding>(geneIndex, integerValue);
    }

    bool DecodeBooleanGene(size_t geneIndex);

    void EncodeBooleanGene(size_t geneIndex, bool value);

    std::byte* GetRawGene(size_t geneIndex, size_t* geneBitWidth);

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

    template <typename FloatType = double, typename IntegerType = uint64_t>
    static FloatType DecodeFloat(IntegerType integerValue, size_t bitWidth, FloatType min, FloatType max) {
        auto localMax = std::numeric_limits<IntegerType>::max() >> (sizeof(IntegerType) * 8 - bitWidth);
        FloatType factor = (FloatType)integerValue / (FloatType)localMax;
        return factor * (max - min) + min;
    }

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
};

} // namespace panga
