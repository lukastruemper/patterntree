#include "cluster.h"

PatternTree::Cluster::Cluster(std::string topology,
    std::vector<std::string> ordered_ids,
    std::vector<double> bandwidth_matrix,
    std::vector<double> latency_matrix,
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes,
    std::unordered_map<std::string, std::string> addresses)
:   topology_(topology),
    ordered_ids_(ordered_ids),
    bandwidth_matrix_(bandwidth_matrix),
    latency_matrix_(latency_matrix),
    nodes_(nodes),
    addresses_(addresses)
{};

std::string PatternTree::Cluster::topology() const
{
    return this->topology_;
};

const std::unordered_map<std::string, std::shared_ptr<PatternTree::Node>>& PatternTree::Cluster::nodes() const
{
    return this->nodes_;
};

double PatternTree::Cluster::bandwidth(const PatternTree::Node& from, const PatternTree::Node& to) const
{
    int pos_from = -1;
    int pos_to = -1;
    for (int i = 0; i < this->ordered_ids_.size(); i++) {
        std::string id = this->ordered_ids_[i];
        if (id == from.identifier()) {
            pos_from = i;
        }
        if (id == to.identifier()) {
            pos_to = i;
        }

        if (pos_to != -1 && pos_from != -1) {
            break;
        }
    }

    int index = pos_from * this->ordered_ids_.size() + pos_to;
    return this->bandwidth_matrix_.at(index);
};

double PatternTree::Cluster::latency(const PatternTree::Node& from, const PatternTree::Node& to) const
{
    int pos_from = -1;
    int pos_to = -1;
    for (int i = 0; i < this->ordered_ids_.size(); i++) {
        std::string id = this->ordered_ids_[i];
        if (id == from.identifier()) {
            pos_from = i;
        }
        if (id == to.identifier()) {
            pos_to = i;
        }

        if (pos_to != -1 && pos_from != -1) {
            break;
        }
    }

    int index = pos_from * this->ordered_ids_.size() + pos_to;
    return this->latency_matrix_.at(index);
};
