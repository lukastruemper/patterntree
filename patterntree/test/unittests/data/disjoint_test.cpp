#pragma once

#include <apt/apt.h>
#include <data/data.h>
#include <data/view.h>

TEST(TestSuiteDisjoint, TestSameData)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto viewA = PatternTree::APT::source<double*>("data", 10);
    auto viewB = PatternTree::View<double*>::full(viewA->data());

    ASSERT_FALSE(viewA->disjoint(*viewA));
    
    ASSERT_FALSE(viewA->disjoint(*viewB));
    ASSERT_FALSE(viewB->disjoint(*viewA));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjoint, TestDifferentData)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto viewA = PatternTree::APT::source<double*>("data", 10);
    auto viewC = PatternTree::APT::source<double*>("data", 10);

    ASSERT_TRUE(viewC->disjoint(*viewA));
    ASSERT_TRUE(viewA->disjoint(*viewC));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjoint, TestDifferentTypes)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto viewA = PatternTree::APT::source<double*>("data", 10);
    auto viewC = PatternTree::APT::source<int*>("data", 10);

    ASSERT_TRUE(viewC->disjoint(*viewA));
    ASSERT_TRUE(viewA->disjoint(*viewC));

    ASSERT_TRUE(std::static_pointer_cast<PatternTree::IView>(viewA)->disjoint(*std::static_pointer_cast<PatternTree::IView>(viewC)));
    ASSERT_TRUE(viewA->disjoint(*std::static_pointer_cast<PatternTree::IView>(viewC)));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjoint, TestElements)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);
    
	auto view = PatternTree::APT::source<double*>("data", 35);
    auto viewA = PatternTree::View<double*>::element(view->data(), 2);
    auto viewB = PatternTree::View<double*>::element(view->data(), 2);
    auto viewC = PatternTree::View<double*>::element(view->data(), 33);
    auto viewD = PatternTree::View<double*>::element(view->data(), 34);

    ASSERT_FALSE(viewA->disjoint(*viewA));
    
    ASSERT_FALSE(viewA->disjoint(*viewB));
    ASSERT_FALSE(viewB->disjoint(*viewA));
    
    ASSERT_TRUE(viewA->disjoint(*viewC));
    ASSERT_TRUE(viewC->disjoint(*viewA));
    ASSERT_TRUE(viewB->disjoint(*viewC));
    ASSERT_TRUE(viewC->disjoint(*viewB));

    ASSERT_FALSE(viewD->disjoint(*viewC));
    ASSERT_FALSE(viewC->disjoint(*viewD));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjoint, TestArbitrarySlices)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("data", 127);
    auto viewA = PatternTree::View<double*>::slice(
        view->data(),
        std::make_pair(1,65)
    );
    auto viewB = PatternTree::View<double*>::slice(
        view->data(),
        std::make_pair(66,92)
    );
    auto viewC = PatternTree::View<double*>::slice(
        view->data(),
        std::make_pair(0,3)
    );

    ASSERT_FALSE(viewA->disjoint(*viewA));
    ASSERT_FALSE(viewA->disjoint(*viewB));
    ASSERT_FALSE(viewB->disjoint(*viewA));
    ASSERT_FALSE(viewA->disjoint(*viewC));
    ASSERT_FALSE(viewC->disjoint(*viewA));
    ASSERT_TRUE(viewB->disjoint(*viewC));
    ASSERT_TRUE(viewC->disjoint(*viewB));

	auto viewNew = PatternTree::APT::source<double*>("data", 127);
    auto viewD = PatternTree::View<double*>::slice(viewNew->data(), std::make_pair(1,8));

    ASSERT_TRUE(viewD->disjoint(*viewA));
    ASSERT_TRUE(viewA->disjoint(*viewD));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}