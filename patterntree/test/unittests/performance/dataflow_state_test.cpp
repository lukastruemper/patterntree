#pragma once

#include <Kokkos_Core.hpp>

#include <data/data.h>
#include <data/view.h>
#include <patterns/map.h>
#include <apt/step.h>
#include <cluster/processor.h>
#include <cluster/team.h>

#include <performance/dataflow_state.h>

#include "../helper.h"

TEST(TestSuiteDataflowState, TestFirstStep)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().begin()->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));

    // BEGIN APT

    PatternTree::APT::initialize(cluster, 2, 32, true);

	auto viewA = PatternTree::APT::source<double*>("fieldA", 36);
	auto viewB = PatternTree::APT::source<double*>("fieldB", 36);

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), viewA);

    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorB), viewB);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // END APT

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& mapA = *(step.begin());
    PatternTree::IPattern& mapB = *(++(step.begin()));

    PatternTree::DataflowState state;
   
    ASSERT_EQ(state.owns(team->processor()).size(), 0);
    ASSERT_EQ(state.owned_by(*viewA).size(), 0);
    ASSERT_EQ(state.owned_by(*viewB).size(), 0);

    step.assign(mapA, team);
    step.assign(mapB, team);

    state.update(step);

    ASSERT_EQ(state.owns(team->processor()).size(), 4);

    auto ownersA = state.owned_by(*viewA);
    auto ownersB = state.owned_by(*viewB);
    
    for (auto const& entry : ownersA)
    {
        ASSERT_EQ(entry.second, &(team->processor()));
    }
    for (auto const& entry : ownersB)
    {
        ASSERT_EQ(entry.second, &(team->processor()));
    }
};

TEST(TestSuiteDataflowState, TestOverwrite)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().begin()->second;
    std::shared_ptr<PatternTree::Processor> processorA = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Processor> processorB = (++(device->processors().begin()))->second;
    std::shared_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 1));
    std::shared_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 1));

    // BEGIN APT

    PatternTree::APT::initialize(cluster, 2, 32, true);

	auto view = PatternTree::APT::source<double*>("fieldA", 36);

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), view);
    
    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorB), view);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // END APT

    PatternTree::Step& stepA = *(apt->begin());
    PatternTree::Step& stepB = *(++(apt->begin()));
    PatternTree::IPattern& mapA = *(stepA.begin());
    PatternTree::IPattern& mapB = *(stepB.begin());

    stepA.assign(mapA, teamA);
    stepB.assign(mapB, teamB);

    PatternTree::DataflowState state;
    state.update(stepA);

    ASSERT_EQ(state.owns(teamA->processor()).size(), 2);
    ASSERT_EQ(state.owns(teamB->processor()).size(), 0);

    auto owners = state.owned_by(*view);    
    for (auto const& entry : owners)
    {
        ASSERT_EQ(entry.second, &(teamA->processor()));
    }

    state.update(stepB);

    ASSERT_EQ(state.owns(teamA->processor()).size(), 0);
    ASSERT_EQ(state.owns(teamB->processor()).size(), 2);

    owners = state.owned_by(*view);    
    for (auto const& entry : owners)
    {
        ASSERT_EQ(entry.second, &(teamB->processor()));
    }
    
};
/*
TEST(TestSuiteDataflowState, TestSharedView)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().begin()->second;
    std::shared_ptr<PatternTree::Processor> processorA = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Processor> processorB = (++(device->processors().begin()))->second;
    std::shared_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 1));
    std::shared_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 1));

    // BEGIN APT

    PatternTree::APT::initialize(cluster, 2, 32, true);

	auto viewA = PatternTree::APT::source<double*>("fieldA", 32);
	auto viewB = PatternTree::APT::source<double*>("fieldB", 32);
	auto viewC = PatternTree::APT::source<double*>("fieldC", 32);

    std::unique_ptr<TwoViewsMapFunctor> functorA(new TwoViewsMapFunctor(viewA));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functorA), viewB);
    
    std::unique_ptr<TwoViewsMapFunctor> functorB(new TwoViewsMapFunctor(viewA));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functorB), viewC);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // END APT

    PatternTree::Step& stepA = *(apt->begin());
    PatternTree::Step& stepB = *(++(apt->begin()));
    PatternTree::IPattern& mapA = *(stepA.begin());
    PatternTree::IPattern& mapB = *(stepB.begin());

    stepA.assign(mapA, teamA);
    stepB.assign(mapB, teamB);

    PatternTree::DataflowState state;
    state.update(stepA);

    ASSERT_EQ(state.owns(teamA->processor()).size(), 2);
    ASSERT_EQ(state.owns(teamB->processor()).size(), 0);

    auto owners = state.owned_by(*view);    
    for (auto const& entry : owners)
    {
        ASSERT_EQ(entry.second, &(teamA->processor()));
    }

    state.update(stepB);

    ASSERT_EQ(state.owns(teamA->processor()).size(), 0);
    ASSERT_EQ(state.owns(teamB->processor()).size(), 2);

    owners = state.owned_by(*view);    
    for (auto const& entry : owners)
    {
        ASSERT_EQ(entry.second, &(teamB->processor()));
    }
    
};*/

TEST(TestSuiteDataflowState, TestSubviews)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processorA = device->processors().find("1")->second;
    std::shared_ptr<PatternTree::Processor> processorB = device->processors().find("2")->second;
    std::shared_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 1));
    std::shared_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 1));

    // BEGIN APT

    PatternTree::APT::initialize(cluster, 2, 32, true);

	auto view = PatternTree::APT::source<double*>("fieldA", 128);
    auto view_lower = PatternTree::View<double*>::slice(view->data(), std::make_pair(0, 64));
    auto view_upper = PatternTree::View<double*>::slice(view->data(), std::make_pair(64, 128));

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), view_lower);
    
    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorB), view_upper);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // END APT

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& mapA = *(step.begin());
    PatternTree::IPattern& mapB = *(++(step.begin()));

    step.assign(mapA, teamA);
    step.assign(mapB, teamB);

    PatternTree::DataflowState state;
    state.update(step);

    int i = 0;
    auto owners = state.owned_by(*view);    
    for (auto const& entry : owners)
    {
        if (entry.first->begins()[0] == 0 || entry.first->begins()[0] == 32) {
            ASSERT_EQ(entry.second, &(teamA->processor()));
        } else {
            ASSERT_EQ(entry.second, &(teamB->processor()));
        }

        i++;
    }
}
