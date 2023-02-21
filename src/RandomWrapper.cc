//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <random>
#include <cstddef>
#include <cstdint>
#include <array>
#include <functional>

#include "RandomWrapper.h"

namespace panga {

RandomWrapper::RandomWrapper() {
    std::array<int, 624> seed_data;
    std::random_device r;
    std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 eng(seq);
    this->engine_ = eng;
}

bool RandomWrapper::CoinFlip(double probability) {
    std::bernoulli_distribution dist{ probability };
    return dist(this->engine_);
}

std::byte RandomWrapper::RandomByte() {
    return static_cast<std::byte>(this->engine_());
}

}  // namespace panga
