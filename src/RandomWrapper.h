//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef RANDOMWRAPPER_H__
#define RANDOMWRAPPER_H__

#include <cstddef>
#include <cstdint>
#include <random>

namespace panga {

class RandomWrapper {
public:
    RandomWrapper();
    RandomWrapper(const RandomWrapper& rhs) = delete;
    RandomWrapper& operator=(const RandomWrapper& rhs) = delete;
    ~RandomWrapper() = default;

    template <typename IntegerType = uint32_t>
    IntegerType RandomInteger(IntegerType min, IntegerType max) {
        std::uniform_int_distribution<IntegerType> dist(min, max);
        return dist(engine_);
    }

    template <typename FloatType = double>
    FloatType RandomFloat(FloatType min, FloatType max) {
        std::uniform_real_distribution<FloatType> dist(min, max);
        return dist(engine_);
    }

    bool CoinFlip(double probability);

    std::byte RandomByte();

private:
    std::mt19937 engine_;
};

}  // namespace panga

#endif  // RANDOMWRAPPER_H__
