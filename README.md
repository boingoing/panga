[![master build status](https://travis-ci.org/boingoing/panga.svg?branch=master)](https://travis-ci.org/boingoing/panga/builds#)

# panga

A simple, portable, and efficient genetic algorithm library.

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
