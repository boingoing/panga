//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#include <vector>

namespace panga {

class Individual;
class RandomWrapper;

class Population : public std::vector<Individual*> {
protected:
    std::vector<double> _partialSums;

public:
    void InitializePartialSums();

    Individual* UniformSelect(RandomWrapper* randomWrapper);
    Individual* RouletteWheelSelect(RandomWrapper* randomWrapper);
    Individual* TournamentSelect(size_t tournamentSize, RandomWrapper* randomWrapper);
    Individual* RankSelect();
};

} // namespace panga
