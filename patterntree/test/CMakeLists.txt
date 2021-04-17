cmake_minimum_required (VERSION 3.16)

project(PatternTreeTest VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

include(FetchContent)
FetchContent_Declare(
    gtest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(gtest)

add_executable(unittests unittests.cpp)
add_executable(algorithms algorithms.cpp)

target_link_libraries(unittests PRIVATE gtest)
target_link_libraries(unittests PRIVATE patterntree)

target_link_libraries(algorithms PRIVATE gtest)
target_link_libraries(algorithms PRIVATE patterntree)
