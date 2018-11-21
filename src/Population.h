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
    Individual* TournamentSelect(size_t _tournamentSize, RandomWrapper* randomWrapper);
    Individual* RankSelect();
};

} // namespace panga
