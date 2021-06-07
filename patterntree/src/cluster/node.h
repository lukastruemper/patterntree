#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <cwalk.h>

#include "cluster/device.h"

using json = nlohmann::json;

namespace PatternTree
{

class Cluster;
class Node {
std::string identifier_;
std::string type_;
std::vector<std::string> ordered_ids_;
std::vector<double> bandwidth_matrix_;
std::vector<double> latency_matrix_;
std::unordered_map<std::string, std::shared_ptr<Device>> devices_;

std::weak_ptr<Cluster> cluster_;

public:
    friend class Cluster;

    Node(std::string type,
        std::vector<std::string> ordered_ids,
        std::vector<double> bandwidth_matrix,
        std::vector<double> latency_matrix,
        std::unordered_map<std::string, 
        std::shared_ptr<Device>> devices
    );

    const Cluster& cluster() const;
    std::string type() const;
    std::string identifier() const;
    const std::unordered_map<std::string, std::shared_ptr<Device>>& devices() const;

    double bandwidth(const Device& from, const Device& to) const;
    double latency(const Device& from, const Device& to) const;

    static std::shared_ptr<Node> parse(std::string path);
};
}

