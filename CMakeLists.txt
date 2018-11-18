cmake_minimum_required (VERSION 2.6)
project (panga_test)

include_directories (${PROJECT_SOURCE_DIR}/src)

set (SOURCES ${PROJECT_SOURCE_DIR}/src/GeneticAlgorithm.cc
	     ${PROJECT_SOURCE_DIR}/src/Genome.cc
	     ${PROJECT_SOURCE_DIR}/src/BitVector.cc
	     ${PROJECT_SOURCE_DIR}/src/Chromosome.cc
	     ${PROJECT_SOURCE_DIR}/src/Individual.cc
	     ${PROJECT_SOURCE_DIR}/src/Population.cc
	     ${PROJECT_SOURCE_DIR}/test/test.cc)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_executable (panga_test ${SOURCES})
