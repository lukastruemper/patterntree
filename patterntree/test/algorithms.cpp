#include <gtest/gtest.h>

#include "algorithms/jacobi.cpp"
#include "algorithms/kmeans.cpp"
#include "algorithms/monte_carlo.cpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
