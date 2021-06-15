#include <cluster/cluster.h>
#include <cluster/node.h>
#include <cluster/device.h>
#include <cluster/processor.h>

TEST(TestSuiteCluster, TestProcessor) {
    std::shared_ptr<PatternTree::Processor> processor = PatternTree::Processor::parse("../clusters/CPU/socket_platinum_8160.json");

    ASSERT_EQ(processor->cores(), 24);
    ASSERT_EQ(processor->arithmetic_units(), 4);
    ASSERT_EQ(processor->frequency(), 2100);
    ASSERT_EQ(processor->cache_size(), 32);
    ASSERT_EQ(processor->cache_latency(), 20.95);
    ASSERT_EQ(processor->cache_bandwidth(), 38000.0);
}

TEST(TestSuiteCluster, TestDevice) {
    std::shared_ptr<const PatternTree::Device> device = PatternTree::Device::parse("../clusters/CPU/cpu_platinum_8160.json");

    ASSERT_EQ(device->type(), "CPU");
    ASSERT_EQ(device->memory_size(), 192000.0);
    ASSERT_EQ(device->memory_latency(), 42.38);
    ASSERT_EQ(device->memory_bandwidth(), 21330.0);
    ASSERT_EQ(device->memory_max_bandwidth(), 97300.0);
    ASSERT_EQ(device->processors().size(), 2);
    for (auto const& processor : device->processors())
    {
        ASSERT_EQ(&(processor.second->device()), device.get());
    }
}

TEST(TestSuiteCluster, TestNode) {
    std::shared_ptr<const PatternTree::Node> node = PatternTree::Node::parse("../clusters/Nodes/node_c18g.json");

    ASSERT_EQ(node->type(), "numa");
    ASSERT_EQ(node->devices().size(), 3);

    auto cpu1 = node->devices().find("CPU1");
    ASSERT_EQ(&(cpu1->second->node()), node.get());
    
    auto gpu1 = node->devices().find("GPU1");
    ASSERT_EQ(&(gpu1->second->node()), node.get());
    
    auto gpu2 = node->devices().find("GPU2");
    ASSERT_EQ(&(gpu2->second->node()), node.get());

    double bandwidth = node->bandwidth(*(cpu1->second), *(gpu1->second));
    ASSERT_EQ(bandwidth, 12152);

    bandwidth = node->bandwidth(*(cpu1->second), *(gpu2->second));
    ASSERT_EQ(bandwidth, 12152);

    bandwidth = node->bandwidth(*(gpu1->second), *(gpu2->second));
    ASSERT_EQ(bandwidth, 12152);

    bandwidth = node->bandwidth(*(gpu1->second), *(cpu1->second));
    ASSERT_EQ(bandwidth, 12152);

    bandwidth = node->bandwidth(*(gpu2->second), *(cpu1->second));
    ASSERT_EQ(bandwidth, 12152);

    bandwidth = node->bandwidth(*(gpu2->second), *(gpu1->second));
    ASSERT_EQ(bandwidth, 12152);

    bandwidth = node->bandwidth(*(cpu1->second), *(cpu1->second));
    ASSERT_EQ(bandwidth, 0);

    bandwidth = node->bandwidth(*(gpu1->second), *(gpu1->second));
    ASSERT_EQ(bandwidth, 0);

    bandwidth = node->bandwidth(*(gpu2->second), *(gpu2->second));
    ASSERT_EQ(bandwidth, 0);

    double latency = node->latency(*(cpu1->second), *(gpu1->second));
    ASSERT_EQ(latency, 7210);

    latency = node->latency(*(cpu1->second), *(gpu2->second));
    ASSERT_EQ(latency, 7210);

    latency = node->latency(*(gpu1->second), *(gpu2->second));
    ASSERT_EQ(latency, 7210);

    latency = node->latency(*(gpu1->second), *(cpu1->second));
    ASSERT_EQ(latency, 7210);

    latency = node->latency(*(gpu2->second), *(cpu1->second));
    ASSERT_EQ(latency, 7210);

    latency = node->latency(*(gpu2->second), *(gpu1->second));
    ASSERT_EQ(latency, 7210);

    latency = node->latency(*(cpu1->second), *(cpu1->second));
    ASSERT_EQ(latency, 0);

    latency = node->latency(*(gpu1->second), *(gpu1->second));
    ASSERT_EQ(latency, 0);

    latency = node->latency(*(gpu2->second), *(gpu2->second));
    ASSERT_EQ(latency, 0);
}

TEST(TestSuiteCluster, TestCluster) {
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");

    ASSERT_EQ(cluster->topology(), "fully");
    ASSERT_EQ(cluster->nodes().size(), 2);

    auto node1 = cluster->nodes().find("Node1");
    ASSERT_EQ(&(node1->second->cluster()), cluster.get());

    auto node2 = cluster->nodes().find("Node2");
    ASSERT_EQ(&(node2->second->cluster()), cluster.get());

    double bandwidth = cluster->bandwidth(*(node1->second), *(node2->second));
    ASSERT_EQ(bandwidth, 4148.0);

    bandwidth = cluster->bandwidth(*(node2->second), *(node1->second));
    ASSERT_EQ(bandwidth, 4148.0);

    bandwidth = cluster->bandwidth(*(node1->second), *(node1->second));
    ASSERT_EQ(bandwidth, 0.0);

    bandwidth = cluster->bandwidth(*(node2->second), *(node2->second));
    ASSERT_EQ(bandwidth, 0.0);

    double latency = cluster->latency(*(node1->second), *(node2->second));
    ASSERT_EQ(latency, 1840.0);

    latency = cluster->latency(*(node2->second), *(node1->second));
    ASSERT_EQ(latency, 1840.0);

    latency = cluster->latency(*(node1->second), *(node1->second));
    ASSERT_EQ(latency, 0.0);

    latency = cluster->latency(*(node2->second), *(node2->second));
    ASSERT_EQ(latency, 0.0);
}

TEST(TestSuiteCluster, TestDistance) {
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");

    std::shared_ptr<PatternTree::Node> nodeA = (cluster->nodes().begin())->second;
    std::shared_ptr<PatternTree::Device> deviceA = (nodeA->devices().begin())->second;
    std::shared_ptr<PatternTree::Device> deviceB = (++(nodeA->devices().begin()))->second;
    std::shared_ptr<PatternTree::Processor> processorA = (deviceA->processors().begin())->second;
    std::shared_ptr<PatternTree::Processor> processorB = (++(deviceA->processors().begin()))->second;
    std::shared_ptr<PatternTree::Processor> processorC = (deviceB->processors().begin())->second;

    std::shared_ptr<PatternTree::Node> nodeB = (++(cluster->nodes().begin()))->second;
    std::shared_ptr<PatternTree::Device> deviceC = (nodeB->devices().begin())->second;
    std::shared_ptr<PatternTree::Processor> processorD = (deviceC->processors().begin())->second;

    ASSERT_EQ(PatternTree::Cluster::distance(*processorA, *processorA), PatternTree::Cluster::Distance::PROCESSOR);
    ASSERT_EQ(PatternTree::Cluster::distance(*processorA, *processorB), PatternTree::Cluster::Distance::DEVICE);
    ASSERT_EQ(PatternTree::Cluster::distance(*processorA, *processorC), PatternTree::Cluster::Distance::NODE);
    ASSERT_EQ(PatternTree::Cluster::distance(*processorA, *processorD), PatternTree::Cluster::Distance::CLUSTER);
}
