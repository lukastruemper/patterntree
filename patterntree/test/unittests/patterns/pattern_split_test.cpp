#pragma once

#include "data/data.h"
#include "data/view.h"
#include "patterns/map.h"
#include "patterns/pattern_split.h"

#include "../helper.h"

TEST(TestSuitePatternSplit, TestConstructDefault)
{
	

	std::shared_ptr<PatternTree::Data<double*>> data(new PatternTree::Data<double*>("field", 1000));
    std::shared_ptr<PatternTree::View<double*>> view = PatternTree::View<double*>::full(data);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    auto map = PatternTree::Map<double*>::create<DummyMapFunctor>(std::move(functor), view, 1);
    std::shared_ptr<PatternTree::Map<double*>> map_ = std::move(map);

    PatternTree::PatternSplit split(map_);
    ASSERT_EQ(&(split.pattern()), map_.get());
    ASSERT_EQ(split.begin(), 0);
    ASSERT_EQ(split.end(), map_->width());
    ASSERT_EQ(split.produces().size(), 1);
    ASSERT_EQ(split.consumes().size(), 1);
    ASSERT_EQ(split.consumes()[0], view);
    ASSERT_EQ(split.produces()[0], view);
};

TEST(TestSuitePatternSplit, TestConstructMultipleViewsDefault)
{
	

	std::shared_ptr<PatternTree::Data<double*>> dataA(new PatternTree::Data<double*>("field", 1000));
    std::shared_ptr<PatternTree::View<double*>> viewA = PatternTree::View<double*>::full(dataA);

    std::shared_ptr<PatternTree::Data<double*>> dataB(new PatternTree::Data<double*>("field", 1000));
    std::shared_ptr<PatternTree::View<double*>> viewB = PatternTree::View<double*>::full(dataB);

    std::unique_ptr<TwoViewsMapFunctor> functor(new TwoViewsMapFunctor(viewB));
    auto map = PatternTree::Map<double*>::create<TwoViewsMapFunctor>(std::move(functor), viewA, 1);
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
};
