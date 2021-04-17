#pragma once

#include <utility>
#include <memory>
#include <vector>

#include <apt/apt.h>
#include <cluster/cluster.h>
#include <patterns/map.h>
#include <data/data.h>
#include <data/view.h>

#include "../helper.h"

TEST(TestSuiteAPT, TestEmptyAPT)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster);
    
    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->sources().size(), 0);
    ASSERT_EQ(apt->size(), 0);
};

TEST(TestSuiteAPT, TestAddSource)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster);

	auto field = PatternTree::APT::source<double*>("field", 2);
    auto data = field->data().lock();

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    ASSERT_EQ(apt->sources().size(), 1);
    ASSERT_EQ(apt->sources()[0], data);
};

TEST(TestSuiteAPT, TestAddPattern)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster);

	auto data = PatternTree::APT::source<double*>("field", 2);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), data);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    auto step = apt->begin();
    auto pattern = step->begin();

    ASSERT_EQ(apt->size(), 1);
    ASSERT_EQ(pattern->produces()[0], data);
};

TEST(TestSuiteDataBasis, TestBasisOneDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto field = PatternTree::APT::source<double*>("field", 513);
    auto data = field->data().lock();
    
    std::vector<std::shared_ptr<PatternTree::IView>> basis = data->basis();
    for (size_t i = 0; i < 17; i++)
    {
        auto basis_view = basis[i];
        ASSERT_EQ(basis_view->begins()[0], i * 32);
        if (i == 16)
        {
            ASSERT_EQ(basis_view->ends()[0], 513);
            ASSERT_EQ(basis_view->shape()[0], 1);
        } else {
            ASSERT_EQ(basis_view->ends()[0], (i + 1) * 32);
            ASSERT_EQ(basis_view->shape()[0], 32);
        }
    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};

TEST(TestSuiteDataBasis, TestBasisTwoDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto field = PatternTree::APT::source<double**>("field", 513, 257);
    auto data = field->data().lock();
    
    std::vector<std::shared_ptr<PatternTree::IView>> basis = data->basis();
    for (size_t i = 0; i < 17; ++i)
    {
        for (size_t j = 0; j < 9; j++)
        {
            auto basis_view = basis[i * 9 + j];
            ASSERT_EQ(basis_view->begins()[1], j * 32);

            if (j == 8)
            {
                ASSERT_EQ(basis_view->ends()[1], 257);
                ASSERT_EQ(basis_view->shape()[1], 1);
            } else {
                ASSERT_EQ(basis_view->ends()[1], (j + 1) * 32);
                ASSERT_EQ(basis_view->shape()[1], 32);
            }

            ASSERT_EQ(basis_view->begins()[0], i * 32);

            if (i == 16)
            {
                ASSERT_EQ(basis_view->ends()[0], 513);
                ASSERT_EQ(basis_view->shape()[0], 1);
            } else {
                ASSERT_EQ(basis_view->ends()[0], (i + 1) * 32);
                ASSERT_EQ(basis_view->shape()[0], 32);
            }
        }
    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
};
