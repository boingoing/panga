[![main build status](https://app.travis-ci.com/boingoing/panga.svg?branch=main)](https://app.travis-ci.com/github/boingoing/panga/builds)

# panga

A zero-dependency, portable, and efficient genetic algorithm library.

## Why panga

Panga is adapted from the genetic algorithm component of a hobby project named PAN built way back in 2008. For the tenth anniversary of the project, panga was split out into its own library and re-implemented in c++17. Panga is the PAN genetic algorithm (PanGa) component.

## Using panga

Panga offers a simple interface for building and managing genetic algorithms.

```c++
// Return fitness score for Individual - lower is better.
double TestObjective(Individual* individual, void* userData) {
    // Cast the user data into a BitVector - this is target.
    auto* bv = static_cast<BitVector*>(userData);
    
    // Calculate fitness as count of bits different from target.
    const auto fail_bits = br->HammingDistance(*individual);

    // Best individual would have zero different bits.
    return static_cast<double>(fail_bits);
}

void Test() {
    // A bit-pattern we want the genetic algorithm to match.
    BitVector target;
    target.FromStringHex("ffffffff", 8);

    GeneticAlgorithm ga;
    Genome& genome = ga.GetGenome();
    // Treat the genome as a bitvector with length equal to the target.
    genome.AddBooleanGenes(target.GetBitCount());

    ga.SetPopulationSize(100);
    ga.SetFitnessFunction(TestObjective);
    ga.SetUserData(&target);
    ga.Initialize();
    
    // When the minimum score of the population drops below 1, the genetic algorithm has matched the bit-pattern.
    double min_score = 1;
    do {
      ga.Step();
      
      // Print current generation stats.
      const auto& population = ga.GetPopulation();
      std::cout << "Generation " << ga.GetCurrentGeneration()
                << " => avg: " << population.GetAverageScore()
                << " min: " << population.GetMinimumScore()
                << " stdev: " << population.GetScoreStandardDeviation()
                << " popdiv: " << population.GetPopulationDiversity()
                << std::endl;
      min_score = population.GetMinimumScore();
    } while (min_score > 0.5);
}
```

## Building panga

You can build panga on any platform with a compiler which supports c++17 language standards mode. The library is designed to be portable and easy to add to your project. We do not release binaries here, but panga compiles into a static library which can be added as a dependency. Add the panga cmake file to your build system and you should be ready to use panga.

### Tested build configurations

Windows 10
* CMake 3.17.0
* Visual Studio 2019 16.11.23

Ubuntu 18.04
* CMake 3.16.3
* Clang 10.0.0

## Testing panga

The library ships with a simple test program in the `panga/test` folder.

```console
> git clone https://github.com/boingoing/panga/panga.git
> mkdir panga/build
> cd panga/build
> cmake ..
> make
> ./panga_test
```

## Documentation

https://boingoing.github.io/panga/html/annotated.html
