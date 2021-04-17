#pragma once

#include <functional>

#include <apt/step.h>
#include <data/data.h>
#include <data/view.h>
#include <patterns/map.h>
#include <cluster/cluster.h>
#include <cluster/processor.h>
#include <cluster/team.h>

#include "../helper.h"

TEST(TestSuiteStepMapping, TestDefaultSplit)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("field", 128);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), view);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // Assertions

    PatternTree::Step& step = *(apt->begin());
    ASSERT_EQ(step.size(), 1);

    PatternTree::IPattern& map = *(step.begin());
    auto splits = step.splits(map);
    ASSERT_EQ(splits.size(), 1);

    const PatternTree::PatternSplit& split = splits[0].get();
    ASSERT_EQ(&(split.pattern()), &map);
    ASSERT_EQ(split.begin(), 0);
    ASSERT_EQ(split.end(), map.width());
    ASSERT_EQ(split.consumes().size(), 1);
    ASSERT_EQ(split.produces().size(), 1);
    ASSERT_EQ(split.produces()[0], view);
}

TEST(TestSuiteStepMapping, TestSplit)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster);

	auto viewA = PatternTree::APT::source<double*>("fieldA", 130);
	auto viewB = PatternTree::APT::source<double*>("fieldB", 130);

    std::unique_ptr<SplitMapFunctor> functor(new SplitMapFunctor(viewB));
    PatternTree::APT::map<double*, SplitMapFunctor>(std::move(functor), viewA);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // Assertions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());

    auto splits = step.split(map, 4);
    ASSERT_EQ(splits.size(), 5);

    for (int i = 0; i < 5; i++) {
        const PatternTree::PatternSplit& split = splits.at(i).get();

        int start = i * 32;
        int end = (i == 4) ? 130 : (i + 1) * 32;

        double flops = 0.0;
        for (int j = start; j < end; j++) {
            flops += j + 1;
        }

        ASSERT_EQ(&(split.pattern()), &map);
        ASSERT_EQ(split.begin(), start);
        ASSERT_EQ(split.end(), end);
        ASSERT_EQ(split.width(), end - start);
        ASSERT_EQ(split.flops(), flops);
        ASSERT_EQ(split.consumes().size(), 2);
        ASSERT_EQ(split.produces().size(), 1);

        auto subviewB = split.consumes()[1];
        ASSERT_EQ(subviewB->begins()[0], start);
        ASSERT_EQ(subviewB->ends()[0], end);

        auto subviewA = split.produces()[0];
        ASSERT_EQ(subviewA->begins()[0], start);
        ASSERT_EQ(subviewA->ends()[0], end);
    }
}

TEST(TestSuiteStepMapping, TestAssignAndFreeSplit)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().begin()->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));
       
    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("field", 128);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), view);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // Assertions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());
    auto splits = step.splits(map);
    const PatternTree::PatternSplit& split = splits[0].get();

    // Initial conditions

    ASSERT_FALSE(step.complete());
    ASSERT_FALSE(step.assigned(split).has_value());
    ASSERT_EQ(step.assigned(*team).size(), 0);

    step.assign(split, team);

    // Assigned conditions

    ASSERT_TRUE(step.complete());
    ASSERT_EQ(step.assigned(split).value(), team);
    ASSERT_EQ(step.assigned(*team).size(), 1);
    ASSERT_EQ(&(step.assigned(*team)[0].get()), &split);

    step.free(split);

    // Initial conditions

    ASSERT_FALSE(step.complete());
    ASSERT_FALSE(step.assigned(split).has_value());
    ASSERT_EQ(step.assigned(*team).size(), 0);
}

TEST(TestSuiteStepMapping, TestAssignAndFreePattern)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().begin()->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));
       
    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("field", 128);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), view);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // Assertions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());
    auto splits = step.splits(map);
    const PatternTree::PatternSplit& split = splits[0].get();

    // Initial conditions

    ASSERT_FALSE(step.complete());
    ASSERT_FALSE(step.assigned(split).has_value());
    ASSERT_EQ(step.assigned(*team).size(), 0);

    step.assign(map, team);

    ASSERT_TRUE(step.complete());
    ASSERT_EQ(step.assigned(split).value(), team);
    ASSERT_EQ(step.assigned(*team).size(), 1);
    ASSERT_EQ(&(step.assigned(*team)[0].get()), &split);

    step.free(map);

    // Initial conditions

    ASSERT_FALSE(step.complete());
    ASSERT_FALSE(step.assigned(split).has_value());
    ASSERT_EQ(step.assigned(*team).size(), 0);
}

TEST(TestSuiteStepMapping, TestAssignAndFreeMultiple)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().begin()->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));
       
    PatternTree::APT::initialize(cluster);

	auto viewA = PatternTree::APT::source<double*>("field", 128);
	auto viewB = PatternTree::APT::source<double*>("field", 128);

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), viewA);

    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorB), viewB);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // Assertions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& mapA = *(step.begin());
    PatternTree::IPattern& mapB = *(++(step.begin()));
    const PatternTree::PatternSplit& splitA = step.splits(mapA)[0].get();
    const PatternTree::PatternSplit& splitB = step.splits(mapB)[0].get();


    ASSERT_FALSE(step.complete());
    ASSERT_FALSE(step.assigned(splitA).has_value());
    ASSERT_FALSE(step.assigned(splitB).has_value());
    ASSERT_EQ(step.assigned(*team).size(), 0);

    step.assign(mapA, team);

    ASSERT_FALSE(step.complete());
    ASSERT_EQ(step.assigned(splitA).value(), team);
    ASSERT_EQ(step.assigned(*team).size(), 1);
    ASSERT_EQ(&(step.assigned(*team)[0].get()), &splitA);

    step.assign(splitB, team);

    ASSERT_TRUE(step.complete());
    ASSERT_EQ(step.assigned(splitA).value(), team);
    ASSERT_EQ(step.assigned(splitB).value(), team);

    std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> assigned = step.assigned(*team);
    ASSERT_EQ(assigned.size(), 2);

    // TODO: std::find

    bool foundA = false;
    bool foundB = false;
    for (auto it = assigned.begin(); it != assigned.end(); it++)
    {
        foundA = foundA || (&(it->get()) == &splitA);
        foundB = foundB || (&(it->get()) == &splitB);
    }

    ASSERT_TRUE(foundA);
    ASSERT_TRUE(foundB);

    step.free(mapB);

    ASSERT_FALSE(step.complete());
    ASSERT_EQ(step.assigned(splitA).value(), team);
    ASSERT_EQ(step.assigned(*team).size(), 1);
    ASSERT_EQ(&(step.assigned(*team)[0].get()), &splitA);

    step.free(splitA);

    ASSERT_FALSE(step.complete());
    ASSERT_FALSE(step.assigned(splitA).has_value());
    ASSERT_FALSE(step.assigned(splitB).has_value());
    ASSERT_EQ(step.assigned(*team).size(), 0);
}
