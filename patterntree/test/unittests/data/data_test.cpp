#pragma once

#include <Kokkos_Core.hpp>

#include <cluster/cluster.h>
#include <data/data.h>

TEST(TestSuiteData, TestDataOneDim)
{
    std::shared_ptr<PatternTree::Data<double*>> data(new PatternTree::Data<double*>("test_data", 10));
    
    ASSERT_TRUE(data->is_symbolic());
    ASSERT_EQ(data->shape().size(), 1);
    ASSERT_EQ(data->shape()[0], 10);
}

TEST(TestSuiteData, TestDataTwoDim)
{
    std::shared_ptr<PatternTree::Data<double**>> data(new PatternTree::Data<double**>("test_data", 3, 2));
    
    ASSERT_TRUE(data->is_symbolic());
    ASSERT_EQ(data->shape().size(), 2);
    ASSERT_EQ(data->shape()[0], 3);
    ASSERT_EQ(data->shape()[1], 2);
}
