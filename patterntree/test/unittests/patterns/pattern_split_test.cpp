#pragma once

#include "apt/apt.h"

#include "data/data.h"
#include "data/view.h"
#include "patterns/map.h"
#include "patterns/pattern_split.h"

#include "../helper.h"

TEST(TestSuitePatternSplit, TestConstructDefault)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 1000);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    auto map = PatternTree::Map<double*>::create<DummyMapFunctor>("dummy", std::move(functor), view, 1);
    std::shared_ptr<PatternTree::Map<double*>> map_ = std::move(map);

    PatternTree::PatternSplit split(map_);
    ASSERT_EQ(&(split.pattern()), map_.get());
    ASSERT_EQ(split.begin(), 0);
    ASSERT_EQ(split.end(), map_->width());
    ASSERT_EQ(split.produces().size(), 1);
    ASSERT_EQ(split.consumes().size(), 1);
    ASSERT_EQ(split.consumes()[0], view);
    ASSERT_EQ(split.produces()[0], view);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuitePatternSplit, TestConstructMultipleViewsDefault)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto viewA = PatternTree::APT::data<double*>("field", 1000);
	auto viewB = PatternTree::APT::data<double*>("field", 1000);

    std::unique_ptr<TwoViewsMapFunctor> functor(new TwoViewsMapFunctor(viewB));
    auto map = PatternTree::Map<double*>::create<TwoViewsMapFunctor>("dummy", std::move(functor), viewA, 1);
    std::shared_ptr<PatternTree::Map<double*>> map_ = std::move(map);

    PatternTree::PatternSplit split(map_);
    ASSERT_EQ(&(split.pattern()), map_.get());
    ASSERT_EQ(split.begin(), 0);
    ASSERT_EQ(split.end(), map_->width());
    ASSERT_EQ(split.produces().size(), 1);
    ASSERT_EQ(split.consumes().size(), 2);
    ASSERT_EQ(split.consumes()[0], viewA);
    ASSERT_EQ(split.consumes()[1], viewB);
    ASSERT_EQ(split.produces()[0], viewA);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};
