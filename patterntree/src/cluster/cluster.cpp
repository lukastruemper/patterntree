#include "cluster.h"

PatternTree::Cluster::Cluster(std::string topology, std::unordered_map<std::string, std::shared_ptr<Node>> nodes, std::unordered_map<std::string, std::string> addresses)
{
    this->topology_ = topology;
    this->nodes_ = nodes;
    this->addresses_ = addresses;
};

std::string PatternTree::Cluster::topology() const
{
    return this->topology_;
}

const std::unordered_map<std::string, std::shared_ptr<PatternTree::Node>>& PatternTree::Cluster::nodes() const
{
    return this->nodes_;
};

double PatternTree::Cluster::bandwidth(const PatternTree::Node& from, const PatternTree::Node& to) const
{
    return 4148.0;
};

double PatternTree::Cluster::latency(const PatternTree::Node& from, const PatternTree::Node& to) const
{
    return 1840.0;
};
