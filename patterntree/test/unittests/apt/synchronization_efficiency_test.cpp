#pragma once

#include <memory>

#include <apt/apt.h>
#include <cluster/cluster.h>

#include "../helper.h"

TEST(TestSuiteSynchronizationEfficiency, TestDependence)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32, true);

	auto data = PatternTree::APT::data<double*>("field", 2);

    std::unique_ptr<DummyMapFunctor> functor1(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor1), data);

    std::unique_ptr<DummyMapFunctor> functor2(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor2), data);
    
    std::unique_ptr<DummyMapFunctor> functor3(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor3), data);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->size(), 3);
};


TEST(TestSuiteSynchronizationEfficiency, TestIndependence)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32, true);

	auto data1 = PatternTree::APT::data<double*>("field", 2);
	auto data2 = PatternTree::APT::data<double*>("field", 2);
	auto data3 = PatternTree::APT::data<double*>("field", 2);

    std::unique_ptr<DummyMapFunctor> functor1(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor1), data1);

    std::unique_ptr<DummyMapFunctor> functor2(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor2), data2);
    
    std::unique_ptr<DummyMapFunctor> functor3(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor3), data3);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    PatternTree::Step& step = *(apt->begin());

    ASSERT_EQ(apt->size(), 1);
    ASSERT_EQ(step.size(), 3);
};

TEST(TestSuiteSynchronizationEfficiency, TestSharedView)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32, true);

	auto data1 = PatternTree::APT::data<double*>("field", 2);
    auto data2 = PatternTree::APT::data<double*>("field", 3);
    auto data3 = PatternTree::APT::data<double*>("field", 3);

    std::unique_ptr<TwoViewsMapFunctor> functor1(new TwoViewsMapFunctor(data3));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functor1), data1);

    std::unique_ptr<TwoViewsMapFunctor> functor2(new TwoViewsMapFunctor(data3));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functor2), data2);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    PatternTree::Step& step = *(apt->begin());

    ASSERT_EQ(apt->size(), 1);
    ASSERT_EQ(step.size(), 2);

    ASSERT_EQ(step.begin()->consumes()[0], data1);
    ASSERT_EQ(step.begin()->consumes()[1], data3);
    ASSERT_EQ((++step.begin())->consumes()[0], data2);
    ASSERT_EQ((++step.begin())->consumes()[1], data3);
};

TEST(TestSuiteSynchronizationEfficiency, TestBubbleUp)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32, true);

	auto data1 = PatternTree::APT::data<double*>("field", 2);
    auto data2 = PatternTree::APT::data<double*>("field", 3);

    std::unique_ptr<DummyMapFunctor> functor1(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor1), data1);

    std::unique_ptr<DummyMapFunctor> functor2(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor2), data1);
    
    std::unique_ptr<DummyMapFunctor> functor3(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor3), data2);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->size(), 2);
    
    PatternTree::Step& step1 = *(apt->begin());
    PatternTree::Step& step2 = *(++(apt->begin()));

    ASSERT_EQ(step1.size(), 2);
    ASSERT_EQ(step2.size(), 1);

    ASSERT_EQ(step1.begin()->consumes()[0], data1);
    ASSERT_EQ((++(step1.begin()))->consumes()[0], data2);

    ASSERT_EQ(step2.begin()->consumes()[0], data1);
};

TEST(TestSuiteSynchronizationEfficiency, TestStopAndGo)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32, true);

	auto data1 = PatternTree::APT::data<double*>("field", 2);
    auto data2 = PatternTree::APT::data<double*>("field", 3);
    auto data3 = PatternTree::APT::data<double*>("field", 3);

    std::unique_ptr<DummyMapFunctor> functor1(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor1), data1);

    PatternTree::APT::synchronization_efficiency(false);

    std::unique_ptr<DummyMapFunctor> functor2(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor2), data2);
    
    PatternTree::APT::synchronization_efficiency(true);
    PatternTree::APT::synchronization_efficiency_length(1);

    std::unique_ptr<DummyMapFunctor> functor3(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor3), data3);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->size(), 2);
    
    PatternTree::Step& step1 = *(apt->begin());
    PatternTree::Step& step2 = *(++(apt->begin()));
    
    ASSERT_EQ(step1.size(), 1);
    ASSERT_EQ(step2.size(), 2);

    ASSERT_EQ(step1.begin()->consumes()[0], data1);
    ASSERT_EQ(step2.begin()->consumes()[0], data2);
    
    ASSERT_EQ((++(step2.begin()))->consumes()[0], data3);
};