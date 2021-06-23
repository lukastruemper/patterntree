cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

include(FetchContent)
set(Kokkos_ENABLE_OPENMP ON CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "") 
FetchContent_Declare(
    kokkos
    GIT_REPOSITORY https://github.com/kokkos/kokkos
    GIT_TAG        3.4.00
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_TAG        v3.9.1
)
FetchContent_Declare(
    Cwalk
    GIT_REPOSITORY https://github.com/likle/cwalk
    GIT_TAG        v1.2.5
)

FetchContent_MakeAvailable(kokkos nlohmann_json Cwalk)

project(PatternTree VERSION 0.1)

set(PATTERNTREE_SOURCES
src/apt/apt.h
src/apt/apt.cpp
src/apt/step.h
src/apt/step.cpp

src/cluster/cluster.h
src/cluster/cluster.cpp
src/cluster/device.h
src/cluster/device.cpp
src/cluster/node.h
src/cluster/node.cpp
src/cluster/processor.h
src/cluster/processor.cpp
src/cluster/team.h
src/cluster/team.cpp

src/data/data_concepts.h
src/data/data.h
src/data/data.cpp
src/data/view.h
src/data/view.cpp
src/data/value.h
src/data/value.cpp
src/data/backend/symbolic_value.h
src/data/backend/symbolic_value.cpp
src/data/backend/kokkos_value.h
src/data/backend/kokkos_value.cpp

src/patterns/pattern.h
src/patterns/pattern.cpp
src/patterns/pattern_split.h
src/patterns/pattern_split.cpp
src/patterns/map.h
src/patterns/map.cpp

src/optimization/optimizer.h

src/performance/dataflow_state.h
src/performance/dataflow_state.cpp
src/performance/performance_model.h
src/performance/roofline_model.h
src/performance/roofline_model.cpp

src/api/arithmetic.h
)

add_library(patterntree STATIC "${PATTERNTREE_SOURCES}")
target_include_directories(patterntree PUBLIC src)

target_link_libraries(patterntree PUBLIC Kokkos::kokkos)
target_link_libraries(patterntree PUBLIC nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(patterntree PUBLIC cwalk)

add_subdirectory(test)
