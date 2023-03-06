cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

find_program (CLANG_TIDY_EXE NAMES "clang-tidy")
if (CLANG_TIDY_EXE)
  set (CMAKE_CXX_CLANG_TIDY clang-tidy)
endif ()

set (CMAKE_CXX_STANDARD 17)

project (panga)

include_directories (${PROJECT_SOURCE_DIR}/src)

set (LIB_SOURCES
  ${PROJECT_SOURCE_DIR}/src/BitVector.cc
  ${PROJECT_SOURCE_DIR}/src/Chromosome.cc
  ${PROJECT_SOURCE_DIR}/src/GeneticAlgorithm.cc
  ${PROJECT_SOURCE_DIR}/src/Genome.cc
  ${PROJECT_SOURCE_DIR}/src/Individual.cc
  ${PROJECT_SOURCE_DIR}/src/Population.cc
  ${PROJECT_SOURCE_DIR}/src/RandomWrapper.cc)
add_library (panga STATIC ${LIB_SOURCES})

set (TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/test.cc)
add_executable (panga_test ${TEST_SOURCES})
target_link_libraries (panga_test panga)

if (MSVC)
  # disable some benign warnings on MSVC
  add_compile_options ("/Wall;/wd4514;/wd4625;/wd4626;/wd5026;/wd5027;/wd5045;/wd4710;/wd4820;")
else ()
  add_compile_options ("-Wall;")
endif ()
