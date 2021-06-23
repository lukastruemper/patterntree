#pragma once

#include <apt/apt.h>

#include <data/backend/symbolic_value.h>
#include <data/backend/kokkos_value.h>

TEST(TestSuiteData, TestOneDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 10);
    auto data = view->data().lock();

    ASSERT_EQ(data->identifier(), "field");
    ASSERT_EQ(data->shape().size(), 1);
    ASSERT_EQ(data->shape()[0], 10);

    auto value = dynamic_cast<PatternTree::SymbolicValue<double*>*>(&(data->value()));
    ASSERT_TRUE(value != NULL);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteData, TestTwoDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double**>("field", 3, 2);
    auto data = view->data().lock();

    ASSERT_EQ(data->identifier(), "field");
    ASSERT_EQ(data->shape().size(), 2);
    ASSERT_EQ(data->shape()[0], 3);
    ASSERT_EQ(data->shape()[1], 2);

    auto value = dynamic_cast<PatternTree::SymbolicValue<double**>*>(&(data->value()));
    ASSERT_TRUE(value != NULL);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteData, TestCompile)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("field", 3, 2);
    auto data = view->data().lock();

    auto symbolic_value = dynamic_cast<PatternTree::SymbolicValue<double*>*>(&(data->value()));
    ASSERT_TRUE(symbolic_value != NULL);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    auto kokkos_value = dynamic_cast<PatternTree::KokkosValue<double*>*>(&(data->value()));
    ASSERT_TRUE(kokkos_value != NULL);
}

/***** DATA: BASIS *****/

TEST(TestSuiteBasisViews, TestOneDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto field = PatternTree::APT::data<double*>("field", 513);
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
}

TEST(TestSuiteBasisViews, TestTwoDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto field = PatternTree::APT::data<double**>("field", 513, 257);
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
}
