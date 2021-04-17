#pragma once

#include <vector>

#include <apt/apt.h>
#include <data/data.h>
#include <data/view.h>
#include <patterns/map.h>
#include <apt/step.h>
#include <cluster/processor.h>
#include <cluster/team.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

#include "../helper.h"

TEST(TestSuiteRooflineExecutionCosts, TestZero)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));

    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("field", 100);
    
    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), view, 100);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());

    PatternTree::RooflineModel model;
    double costs = model.execution_costs(step.splits(map), *team);

    ASSERT_EQ(costs, 0.0);
};

TEST(TestSuiteRooflineExecutionCosts, TestConstant)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));

    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("field", 100);

    std::unique_ptr<ConstantCostsMapFunctor> functor(new ConstantCostsMapFunctor());
    PatternTree::APT::map<double*, ConstantCostsMapFunctor>(std::move(functor), view, 100);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());
    
    PatternTree::RooflineModel model;
    double costs = model.execution_costs(step.splits(map), *team);

    double expected_costs = 100 * 1;
    expected_costs /= processor->frequency() * PatternTree::IPerformanceModel::FREQUENCY_TO_SECONDS; 
    ASSERT_EQ(costs, expected_costs);
};

TEST(TestSuiteRooflineExecutionCosts, TestTriangle)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));

    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double**>("field", 100, 2);

    std::unique_ptr<TriangleCostsMapFunctor> functor(new TriangleCostsMapFunctor());
    PatternTree::APT::map<double**, TriangleCostsMapFunctor>(std::move(functor), view, 100);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());

    PatternTree::RooflineModel model;
    double costs = model.execution_costs(step.splits(map), *team);

    double expected_costs = (2 + 200) * 50;
    expected_costs /= processor->frequency() * PatternTree::IPerformanceModel::FREQUENCY_TO_SECONDS; 
    ASSERT_EQ(costs, expected_costs);
};


TEST(TestSuiteRooflineNetworkCosts, TestInitial)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));

    PatternTree::APT::initialize(cluster);

	auto view = PatternTree::APT::source<double*>("field", 100);

    std::unique_ptr<DummyMapFunctor> functor(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functor), view, 100);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& step = *(apt->begin());
    PatternTree::IPattern& map = *(step.begin());

    PatternTree::RooflineModel model;
    double costs = model.network_costs(step.splits(map), *team);

    double expected_costs = processor->device().memory_latency() / PatternTree::RooflineModel::LATENCY_TO_SECONDS;
    expected_costs += ((100.0 * 8.0) / 1000.0) / (processor->device().memory_bandwidth() * PatternTree::RooflineModel::BANDWIDTH_TO_SECONDS);
    ASSERT_EQ(costs, expected_costs);
};


TEST(TestSuiteRooflineNetworkCosts, TestCache)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;

    std::shared_ptr<PatternTree::Team> teamA(new PatternTree::Team(processor, 1));
    std::shared_ptr<PatternTree::Team> teamB(new PatternTree::Team(processor, 1));

    PatternTree::APT::initialize(cluster, 2, 32, true);

	auto view = PatternTree::APT::source<double*>("field", 100);

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), view, 100);

    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorB), view, 100);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& stepA = *(apt->begin());
    PatternTree::IPattern& mapA = *(stepA.begin());
    stepA.assign(mapA, teamA);

    PatternTree::RooflineModel model;
    model.update(stepA);

    PatternTree::Step& stepB = *(++(apt->begin()));
    PatternTree::IPattern& mapB = *(stepB.begin());

    double costs = model.network_costs(stepB.splits(mapB), *teamB);

    double expected_costs = processor->cache_latency() / PatternTree::RooflineModel::LATENCY_TO_SECONDS;
    expected_costs += ((100 * 8.0) / 1000.0) / (processor->cache_bandwidth() * PatternTree::RooflineModel::BANDWIDTH_TO_SECONDS);
    ASSERT_EQ(costs, expected_costs);
};

TEST(TestSuiteRooflineNetworkCosts, TestMainMemory)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processorA = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Processor> processorB = (++device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 1));
    std::shared_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 1));

    PatternTree::APT::initialize(cluster, 2, 32, true);

	auto view = PatternTree::APT::source<double*>("field", 100);

    std::unique_ptr<DummyMapFunctor> functorA(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorA), view, 100);

    std::unique_ptr<DummyMapFunctor> functorB(new DummyMapFunctor());
    PatternTree::APT::map<double*, DummyMapFunctor>(std::move(functorB), view, 100);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& stepA = *(apt->begin());
    PatternTree::IPattern& mapA = *(stepA.begin());
    stepA.assign(mapA, teamA);

    PatternTree::RooflineModel model;
    model.update(stepA);

    PatternTree::Step& stepB = *(++(apt->begin()));
    PatternTree::IPattern& mapB = *(stepB.begin());

    double costs = model.network_costs(stepB.splits(mapB), *teamB);

    double expected_costs = device->memory_latency() / PatternTree::RooflineModel::LATENCY_TO_SECONDS;
    expected_costs += ((100.0 * 8.0) / 1000.0) / (device->memory_bandwidth() * PatternTree::RooflineModel::BANDWIDTH_TO_SECONDS);
    ASSERT_EQ(costs, expected_costs);
};

TEST(TestSuiteRooflineNetworkCosts, TestSharedData)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    std::shared_ptr<PatternTree::Node> node = cluster->nodes().begin()->second;
    std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
    std::shared_ptr<PatternTree::Processor> processor = (device->processors().begin())->second;
    std::shared_ptr<PatternTree::Team> team(new PatternTree::Team(processor, 1));

    PatternTree::APT::initialize(cluster);

	auto viewA = PatternTree::APT::source<double*>("field", 8192 * 8192);
	auto viewB = PatternTree::APT::source<double*>("field", 512);
	auto viewC = PatternTree::APT::source<double*>("field", 512);

    std::unique_ptr<TwoViewsMapFunctor> functorA(new TwoViewsMapFunctor(viewA));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functorA), viewB, 2);

    std::unique_ptr<TwoViewsMapFunctor> functorB(new TwoViewsMapFunctor(viewA));
    PatternTree::APT::map<double*, TwoViewsMapFunctor>(std::move(functorB), viewC, 2);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();
    
    // Conditions

    PatternTree::Step& step = *(apt->begin());

    PatternTree::RooflineModel model;
    double costs = model.network_costs(step.splits(), *team);

    double bandwidth = (team->cores() * processor->device().memory_bandwidth() * PatternTree::RooflineModel::BANDWIDTH_TO_SECONDS);
    double expected_costs = processor->device().memory_latency() / PatternTree::RooflineModel::LATENCY_TO_SECONDS;
    expected_costs += (viewA->kbytes() + viewB->kbytes() + viewC->kbytes()) / bandwidth;

    ASSERT_NEAR(costs, expected_costs, 0.000001);
};
