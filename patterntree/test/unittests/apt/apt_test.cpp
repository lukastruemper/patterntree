#pragma once

#include <utility>
#include <memory>
#include <vector>

#include <apt/apt.h>

#include "../helper.h"

TEST(TestSuiteAPT, TestEmptyAPT)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);
    
    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->size(), 0);
    ASSERT_EQ(apt->data().size(), 0);
}

TEST(TestSuiteAPT, TestAddData)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto field = PatternTree::APT::data<double*>("field", 2);
    auto data = field->data().lock();

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->data().size(), 1);
    ASSERT_EQ(apt->data()[0], data);
}

TEST(TestSuiteAPT, TestAddPattern)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto data = PatternTree::APT::data<double*>("field", 2);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), data);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    auto step = apt->begin();
    auto pattern = step->begin();

    ASSERT_EQ(apt->size(), 1);
    ASSERT_EQ(pattern->produces()[0], data);
}
