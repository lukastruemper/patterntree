#pragma once

#include <apt/apt.h>

TEST(TestSuiteView, TestOneDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double*>("field", 96);
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

TEST(TestSuiteView, TestTwoDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double**>("field", 66, 77);
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

/***** VIEW: SUBIVEWS *****/

TEST(TestSuiteSubviews, TestElementOneDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double*>("field", 64);
    auto data = view->data().lock();
    auto element = PatternTree::View<double*>::element(data, 1);

    ASSERT_EQ(element->shape().size(), 1);
    ASSERT_EQ(element->shape()[0], 1);

    auto basis = PatternTree::IView::as_basis(*element);

    ASSERT_EQ(basis.size(), 1);
    ASSERT_TRUE(basis.find(data->basis()[0]) != basis.end()); 

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteSubviews, TestElementTwoDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double**>("field", 121, 33);
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

TEST(TestSuiteSubviews, TestSliceOneDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double*>("field", 96);
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

TEST(TestSuiteSubviews, TestSliceTwoDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double**>("field", 96, 35);
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

TEST(TestSuiteSubviews, TestJoinOneDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double*>("field", 121);
    auto data = view->data().lock();
    
    auto subviewA = PatternTree::View<double*>::slice(data, std::make_pair(1, 2));
    auto subviewB = PatternTree::View<double*>::slice(data, std::make_pair(120, 121));

    auto joined = PatternTree::View<double*>::join(*subviewA, *subviewB);

    ASSERT_EQ(joined->shape().size(), 1);
    ASSERT_EQ(joined->shape()[0], 120);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteSubviews, TestJoinTwoDimensional)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto view = PatternTree::APT::data<double**>("field", 121, 33);
    auto data = view->data().lock();
    
    auto subviewA = PatternTree::View<double**>::slice(data, std::make_pair(1, 2), std::make_pair(0, 12));
    auto subviewB = PatternTree::View<double**>::slice(data, std::make_pair(120, 121), std::make_pair(11, 24));

    auto joined = PatternTree::View<double**>::join(*subviewA, *subviewB);

    ASSERT_EQ(joined->shape().size(), 2);
    ASSERT_EQ(joined->shape()[0], 120);
    ASSERT_EQ(joined->shape()[1], 24);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

/***** VIEW: Disjointness *****/

TEST(TestSuiteDisjointViews, TestSameData)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto viewA = PatternTree::APT::data<double*>("data", 10);
    auto viewB = PatternTree::View<double*>::full(viewA->data());

    ASSERT_FALSE(viewA->disjoint(*viewA));
    
    ASSERT_FALSE(viewA->disjoint(*viewB));
    ASSERT_FALSE(viewB->disjoint(*viewA));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjointViews, TestDifferentData)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto viewA = PatternTree::APT::data<double*>("data", 10);
    auto viewC = PatternTree::APT::data<double*>("data", 10);

    ASSERT_TRUE(viewC->disjoint(*viewA));
    ASSERT_TRUE(viewA->disjoint(*viewC));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjointViews, TestDifferentTypes)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);

	auto viewA = PatternTree::APT::data<double*>("data", 10);
    auto viewC = PatternTree::APT::data<int*>("data", 10);

    ASSERT_TRUE(viewC->disjoint(*viewA));
    ASSERT_TRUE(viewA->disjoint(*viewC));

    ASSERT_TRUE(std::static_pointer_cast<PatternTree::IView>(viewA)->disjoint(*std::static_pointer_cast<PatternTree::IView>(viewC)));
    ASSERT_TRUE(viewA->disjoint(*std::static_pointer_cast<PatternTree::IView>(viewC)));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}

TEST(TestSuiteDisjointViews, TestElements)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 32);
    
	auto view = PatternTree::APT::data<double*>("data", 35);
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

TEST(TestSuiteDisjointViews, TestArbitrarySlices)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster);

	auto view = PatternTree::APT::data<double*>("data", 127);
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

	auto viewNew = PatternTree::APT::data<double*>("data", 127);
    auto viewD = PatternTree::View<double*>::slice(viewNew->data(), std::make_pair(1,8));

    ASSERT_TRUE(viewD->disjoint(*viewA));
    ASSERT_TRUE(viewA->disjoint(*viewD));

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
}
