#pragma once

#include <apt/apt.h>

/***** VALUE: Symbolic *****/

TEST(TestSuiteSymbolicValue, TestOperatorRead)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 10);

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(view->operator()(i), 0);
    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

/***** VALUE: Kokkos *****/

TEST(TestSuiteKokkosValue, TestOperatorRead)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 10);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(view->operator()(i), 1);
    }
}