[![master build status](https://travis-ci.org/boingoing/panga.svg?branch=master)](https://travis-ci.org/boingoing/panga/builds#)

# panga

A simple, portable, and efficient genetic algorithm library.

## Why panga

Panga is adapted from the genetic algorithm component of a hobby project named PAN built way back in 2008. For the tenth anniversary of the project, panga was split out into its own library and re-implemented in c++17. Panga is the PAN genetic algorithm (PanGa) component.

## Using panga

Panga offers a simple interface for building and managing genetic algorithms.

```c++
using namespace panga;

// Return fitness score for Individual - lower is better.
double TestObjective(Individual* individual, void* userData) {
    // Cast the user data into a BitVector - this is target.
    BitVector* bv = static_cast<BitVector*>(userData);
    
    // Calculate fitness as count of bits different from target.
    size_t failBits = br->HammingDistance(individual);

    return (double) failBits;
}

void Test() {
    // A bit-pattern we want the genetic algorithm to match.
    BitVector target;
    target.FromStringHex("ffffffff", 8);

    // Treat the genome as a bitvector with length equal to the target.
    Genome genome;
    genome.AddBooleanGenes(target.GetBitCount());

    GeneticAlgorithm ga;
    ga.SetGenome(&genome);
    ga.SetPopulationSize(100);
    ga.SetFitnessFunction(TestObjective);
    ga.SetUserData(&target);
    ga.Initialize();
    
    do {
      ga.Step();
      
      // Print current generation stats.
      std::cout << "Generation " << ga.GetCurrentGeneration()
                << " => avg: " << ga.GetAverageScore()
                << " min: " << ga.GetMinimumScore()
                << " stdev: " << ga.GetScoreStandardDeviation()
                << " popdiv: " << ga.GetPopulationDiversity()
                << std::endl;
    } while (ga.GetMinimumScore() > 0.5);
}
```

## Building panga

You can build panga on any platform with a compiler which supports c++17 language standards mode. The library is designed to be portable and easy to add to your project. Add the panga source files in `panga/src` to your build definition and you should be ready to use panga.

### Tested build configurations

Windows 10
* CMake 3.13.0-rc3
* Visual Studio 2017 15.8.9

Ubuntu 18.04
* CMake 3.10.2
* Clang 6.0.0

## Testing panga

The library ships with a simple test program in the `panga/test` folder.

```console
> git clone https://github.com/boingoing/panga/panga.git
> cd panga/out
> cmake ..
> make
> ./panga_test
```

### Using Visual Studio on Windows

Above `cmake` command generates a Visual Studio solution file (`panga/out/panga_test.sln`) on Windows platforms with Visual Studio. You can open this solution in Visual Studio and use it to build the test program.

## Documentation

https://boingoing.github.io/panga/html/annotated.html
