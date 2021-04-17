#include <gtest/gtest.h>

#include "unittests/cluster/cluster_test.cpp"

#include "unittests/data/data_test.cpp"
#include "unittests/apt/apt_test.cpp"
#include "unittests/data/view_test.cpp"
#include "unittests/data/disjoint_test.cpp"

#include "unittests/patterns/map_test.cpp"
#include "unittests/patterns/pattern_split_test.cpp"
#include "unittests/apt/step_mapping_test.cpp"
#include "unittests/apt/happens_before_test.cpp"
#include "unittests/apt/synchronization_efficiency_test.cpp"

#include "unittests/performance/dataflow_state_test.cpp"
#include "unittests/performance/roofline_model_test.cpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
