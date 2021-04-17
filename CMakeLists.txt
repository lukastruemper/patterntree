cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

enable_testing()

project(PatternTree VERSION 1.0)
add_subdirectory(patterntree)
add_subdirectory(samples)

add_test(unittests patterntree/test/unittests)
add_test(algorithms patterntree/test/algorithms)