#pragma once

#include <apt/step.h>
#include <data/data.h>
#include <data/view.h>
#include <patterns/map.h>
#include <cluster/cluster.h>

#include "../helper.h"

TEST(TestSuiteHappensBefore, TestIndependent)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto viewA = PatternTree::APT::data<double*>("fieldA", 512);
    auto viewC = PatternTree::APT::data<double*>("fieldB", 256);

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), viewA);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    std::unique_ptr<DummyMapFunctor> functorC(new DummyMapFunctor());
    auto mapC = PatternTree::Map<double*>::create<DummyMapFunctor>("dummy", std::move(functorC), viewC, 1);
    
    PatternTree::Step& step = *(apt->begin());

    ASSERT_FALSE(step.happensBefore(*mapC));
};

TEST(TestSuiteHappensBefore, TestReadAfterWrite)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto viewA = PatternTree::APT::data<double*>("fieldA", 512);
	auto viewB = PatternTree::View<double*>::slice(viewA->data(), std::make_pair(64, 66));

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), viewA);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    auto mapB = PatternTree::Map<double*>::create<DummyMapFunctor>("dummy", std::move(functorB), viewB, 1);

    PatternTree::Step& step = *(apt->begin());

    ASSERT_TRUE(step.happensBefore(*mapB));
};

TEST(TestSuiteHappensBefore, TestReadAfterRead)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto viewA = PatternTree::APT::data<double*>("fieldA", 512);
	auto viewB = PatternTree::APT::data<double*>("fieldB", 512);
    auto viewC = PatternTree::APT::data<double*>("fieldC", 512);

    std::unique_ptr<TwoViewsMapFunctor> functorA(new TwoViewsMapFunctor(viewC));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functorA), viewA);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    std::unique_ptr<TwoViewsMapFunctor> functorB(new TwoViewsMapFunctor(viewC));
    auto mapB = PatternTree::Map<double*>::create<TwoViewsMapFunctor>("dummy", std::move(functorB), viewB, 1);
    
    PatternTree::Step& step = *(apt->begin());

    ASSERT_FALSE(step.happensBefore(*mapB));
};