#pragma once

#include <apt/apt.h>
#include <data/data.h>
#include <data/view.h>

TEST(TestSuiteView, TestViewOneDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double*>("field", 96);
    auto data = view->data().lock();

    ASSERT_EQ(data->shape().size(), 1);
    ASSERT_EQ(data->shape()[0], 96);

    ASSERT_EQ(view->shape().size(), 1);
    ASSERT_EQ(view->shape()[0], 96);

    auto basis = PatternTree::IView::as_basis(*view);

    ASSERT_EQ(basis.size(), 3);
    for (auto const& basis_view : data->basis())
    {
        ASSERT_TRUE(basis.find(basis_view) != basis.end());
    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewTwoDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double**>("field", 66, 77);
    auto data = view->data().lock();

    ASSERT_EQ(data->shape().size(), 2);
    ASSERT_EQ(data->shape()[0], 66);
    ASSERT_EQ(data->shape()[1], 77);

    ASSERT_EQ(view->shape().size(), 2);
    ASSERT_EQ(view->shape()[0], 66);
    ASSERT_EQ(view->shape()[1], 77);

    auto basis = PatternTree::IView::as_basis(*view);

    ASSERT_EQ(basis.size(), 9);
    for (auto const& basis_view : data->basis())
    {
        ASSERT_TRUE(basis.find(basis_view) != basis.end());
    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewElementOneDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double*>("field", 64);
    auto data = view->data().lock();
    auto element = PatternTree::View<double*>::element(data, 1);

    ASSERT_EQ(element->shape().size(), 1);
    ASSERT_EQ(element->shape()[0], 1);

    auto basis = PatternTree::IView::as_basis(*element);

    ASSERT_EQ(basis.size(), 1);
    ASSERT_TRUE(basis.find(data->basis()[0]) != basis.end()); 

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewElementTwoDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double**>("field", 121, 33);
    auto data = view->data().lock();
    auto element = PatternTree::View<double**>::element(data, 56);

    ASSERT_EQ(element->shape().size(), 2);
    ASSERT_EQ(element->shape()[0], 1);
    ASSERT_EQ(element->shape()[1], 33);

    auto basis = PatternTree::IView::as_basis(*element);

    ASSERT_EQ(basis.size(), 2);
    ASSERT_TRUE(basis.find(data->basis()[2]) != basis.end());
    ASSERT_TRUE(basis.find(data->basis()[3]) != basis.end());

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewSliceOneDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double*>("field", 96);
    auto data = view->data().lock();
    auto slice = PatternTree::View<double*>::slice(data, std::make_pair(0, 33));

    ASSERT_EQ(slice->shape().size(), 1);
    ASSERT_EQ(slice->shape()[0], 33);

    auto basis = PatternTree::IView::as_basis(*slice);

    ASSERT_EQ(basis.size(), 2);
    ASSERT_TRUE(basis.find(data->basis()[0]) != basis.end()); 
    ASSERT_TRUE(basis.find(data->basis()[1]) != basis.end()); 

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewSliceTwoDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double**>("field", 96, 35);
    auto data = view->data().lock();
    auto slice = PatternTree::View<double**>::slice(data, std::make_pair(0, 33), std::make_pair(0, 32));

    ASSERT_EQ(slice->shape().size(), 2);
    ASSERT_EQ(slice->shape()[0], 33);
    ASSERT_EQ(slice->shape()[1], 32);

    auto basis = PatternTree::IView::as_basis(*slice);

    ASSERT_EQ(basis.size(), 2);
    ASSERT_TRUE(basis.find(data->basis()[0]) != basis.end()); 
    ASSERT_TRUE(basis.find(data->basis()[2]) != basis.end()); 

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewJoinOneDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double*>("field", 121);
    auto data = view->data().lock();
    
    auto subviewA = PatternTree::View<double*>::slice(data, std::make_pair(1, 2));
    auto subviewB = PatternTree::View<double*>::slice(data, std::make_pair(120, 121));

    auto joined = PatternTree::View<double*>::join(*subviewA, *subviewB);

    ASSERT_EQ(joined->shape().size(), 1);
    ASSERT_EQ(joined->shape()[0], 120);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteView, TestViewJoinTwoDim)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 2, 32);

	auto view = PatternTree::APT::source<double**>("field", 121, 33);
    auto data = view->data().lock();
    
    auto subviewA = PatternTree::View<double**>::slice(data, std::make_pair(1, 2), std::make_pair(0, 12));
    auto subviewB = PatternTree::View<double**>::slice(data, std::make_pair(120, 121), std::make_pair(11, 24));

    auto joined = PatternTree::View<double**>::join(*subviewA, *subviewB);

    ASSERT_EQ(joined->shape().size(), 2);
    ASSERT_EQ(joined->shape()[0], 120);
    ASSERT_EQ(joined->shape()[1], 24);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}
