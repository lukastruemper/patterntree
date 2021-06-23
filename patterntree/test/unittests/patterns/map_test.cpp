#pragma once

#include <apt/apt.h>

#include <data/view.h>
#include <patterns/map.h>
#include <api/arithmetic.h>

#include "../helper.h"

TEST(TestSuiteMapDataflow, TestSimple)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 1000);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    auto map = PatternTree::Map<double*>::create<DummyMapFunctor>("dummy", std::move(functor), view, 1);

    ASSERT_EQ(map->consumes().size(), 1);
    ASSERT_EQ(map->consumes().at(0), view);

    ASSERT_EQ(map->produces().size(), 1);
    ASSERT_EQ(map->produces().at(0), view);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile(); 
};

TEST(TestSuiteMapDataflow, TestAdditionalView)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto viewA = PatternTree::APT::data<double*>("field", 1000);
	auto viewB = PatternTree::APT::data<double*>("field", 1000);

    std::unique_ptr<TwoViewsMapFunctor> functor(new TwoViewsMapFunctor(viewB));
    auto map = PatternTree::Map<double*>::create<TwoViewsMapFunctor>("dummy", std::move(functor), viewA, 1);

    ASSERT_EQ(map->consumes().size(), 2);
    ASSERT_EQ(map->consumes().at(0), viewA);
    ASSERT_EQ(map->consumes().at(1), viewB);

    ASSERT_EQ(map->produces().size(), 1);
    ASSERT_EQ(map->produces().at(0), viewA);  

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuiteMapFLOPS, TestEmpty)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 1000);

    std::unique_ptr<TemplateMapFunctor<double*>> functor(new TemplateMapFunctor<double*>());
    auto map = PatternTree::Map<double*>::create<TemplateMapFunctor<double*>>("dummy", std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 0);
    ASSERT_EQ(map->flops(999, false), 0);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuiteMapFLOPS, TestConstant)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 1000);

    std::unique_ptr<ConstantCostsMapFunctor> functor(new ConstantCostsMapFunctor());
    auto map = PatternTree::Map<double*>::create<ConstantCostsMapFunctor>("dummy", std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 1);
    ASSERT_EQ(map->flops(999, false), 1); 

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuiteMapFLOPS, TestTriangle)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double**>("field", 100, 2);

    std::unique_ptr<TriangleCostsMapFunctor> functor(new TriangleCostsMapFunctor());
    auto map = PatternTree::Map<double**>::create<TriangleCostsMapFunctor>("dummy", std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 2);
    ASSERT_EQ(map->flops(99, false), 200);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuiteMapFLOPS, TestTouch)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double**>("field", 100, 2);

    std::unique_ptr<CustomTriangleCostsMapFunctor> functor(new CustomTriangleCostsMapFunctor());
    auto map = PatternTree::Map<double**>::create<CustomTriangleCostsMapFunctor>("dummy", std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 3);
    ASSERT_EQ(map->flops(9, true), 21);
    ASSERT_EQ(map->flops(99, false), 201);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuiteMapFLOPS, TestInterpolate)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double**>("field", 100, 2);

    std::unique_ptr<CustomTriangleCostsMapFunctor> functor(new CustomTriangleCostsMapFunctor());
    auto map = PatternTree::Map<double**>::create<CustomTriangleCostsMapFunctor>("dummy", std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 3);
    ASSERT_EQ(map->flops(1, false), 5);
    ASSERT_EQ(map->flops(2, false), 7);
    ASSERT_EQ(map->flops(49, false), 101);
    ASSERT_EQ(map->flops(97, false), 197);
    ASSERT_EQ(map->flops(98, false), 199);
    ASSERT_EQ(map->flops(99, false), 201);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};
