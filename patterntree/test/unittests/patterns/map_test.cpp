#pragma once

#include <Kokkos_Core.hpp>

#include <data/data.h>
#include <data/view.h>
#include <patterns/map.h>
#include <api/arithmetic.h>

#include "../helper.h"

TEST(TestSuiteMapDataflow, TestSimple)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double*>> data(new PatternTree::Data<double*>("field", 1000));
    std::shared_ptr<PatternTree::View<double*>> view = PatternTree::View<double*>::full(data);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    auto map = PatternTree::Map<double*>::create<DummyMapFunctor>(std::move(functor), view, 1);

    ASSERT_EQ(map->consumes().size(), 1);
    ASSERT_EQ(map->consumes().at(0), view);

    ASSERT_EQ(map->produces().size(), 1);
    ASSERT_EQ(map->produces().at(0), view);  
};

TEST(TestSuiteMapDataflow, TestAdditionalView)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double*>> fieldA(new PatternTree::Data<double*>("fieldA", 1000));
    std::shared_ptr<PatternTree::Data<double*>> fieldB(new PatternTree::Data<double*>("fieldB", 1000));

    std::shared_ptr<PatternTree::View<double*>> viewA = PatternTree::View<double*>::full(fieldA);
    std::shared_ptr<PatternTree::View<double*>> viewB = PatternTree::View<double*>::full(fieldB);

    std::unique_ptr<TwoViewsMapFunctor> functor(new TwoViewsMapFunctor(viewB));
    auto map = PatternTree::Map<double*>::create<TwoViewsMapFunctor>(std::move(functor), viewA, 1);

    ASSERT_EQ(map->consumes().size(), 2);
    ASSERT_EQ(map->consumes().at(0), viewA);
    ASSERT_EQ(map->consumes().at(1), viewB);

    ASSERT_EQ(map->produces().size(), 1);
    ASSERT_EQ(map->produces().at(0), viewA);  
};

TEST(TestSuiteMapFLOPS, TestEmpty)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double*>> data(new PatternTree::Data<double*>("field", 1000));
    std::shared_ptr<PatternTree::View<double*>> view = PatternTree::View<double*>::full(data);

    std::unique_ptr<TemplateMapFunctor<double*>> functor(new TemplateMapFunctor<double*>());
    auto map = PatternTree::Map<double*>::create<TemplateMapFunctor<double*>>(std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 0);
    ASSERT_EQ(map->flops(999, false), 0); 
};

TEST(TestSuiteMapFLOPS, TestConstant)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double*>> data(new PatternTree::Data<double*>("field", 1000));
    std::shared_ptr<PatternTree::View<double*>> view = PatternTree::View<double*>::full(data);

    std::unique_ptr<ConstantCostsMapFunctor> functor(new ConstantCostsMapFunctor());
    auto map = PatternTree::Map<double*>::create<ConstantCostsMapFunctor>(std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 1);
    ASSERT_EQ(map->flops(999, false), 1); 
};

TEST(TestSuiteMapFLOPS, TestTriangle)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double**>> data(new PatternTree::Data<double**>("field", 100, 2));
    std::shared_ptr<PatternTree::View<double**>> view = PatternTree::View<double**>::full(data);

    std::unique_ptr<TriangleCostsMapFunctor> functor(new TriangleCostsMapFunctor());
    auto map = PatternTree::Map<double**>::create<TriangleCostsMapFunctor>(std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 2);
    ASSERT_EQ(map->flops(99, false), 200);
};

TEST(TestSuiteMapFLOPS, TestTouch)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double**>> data(new PatternTree::Data<double**>("field", 100, 2));
    std::shared_ptr<PatternTree::View<double**>> view = PatternTree::View<double**>::full(data);

    std::unique_ptr<CustomTriangleCostsMapFunctor> functor(new CustomTriangleCostsMapFunctor());
    auto map = PatternTree::Map<double**>::create<CustomTriangleCostsMapFunctor>(std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 3);
    ASSERT_EQ(map->flops(9, true), 21);
    ASSERT_EQ(map->flops(99, false), 201);
};

TEST(TestSuiteMapFLOPS, TestInterpolate)
{
	Kokkos::initialize();

	std::shared_ptr<PatternTree::Data<double**>> data(new PatternTree::Data<double**>("field", 100, 2));
    std::shared_ptr<PatternTree::View<double**>> view = PatternTree::View<double**>::full(data);

    std::unique_ptr<CustomTriangleCostsMapFunctor> functor(new CustomTriangleCostsMapFunctor());
    auto map = PatternTree::Map<double**>::create<CustomTriangleCostsMapFunctor>(std::move(functor), view, 1);

    ASSERT_EQ(map->flops(0, false), 3);
    ASSERT_EQ(map->flops(1, false), 5);
    ASSERT_EQ(map->flops(2, false), 7);
    ASSERT_EQ(map->flops(49, false), 101);
    ASSERT_EQ(map->flops(97, false), 197);
    ASSERT_EQ(map->flops(98, false), 199);
    ASSERT_EQ(map->flops(99, false), 201);
};
