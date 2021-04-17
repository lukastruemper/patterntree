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
std::string type_;
std::unordered_map<std::string, std::shared_ptr<Device>> devices_;

std::weak_ptr<Cluster> cluster_;

public:
    Node(std::string, std::unordered_map<std::string, std::shared_ptr<Device>>);

    std::string type() const;
    const std::unordered_map<std::string, std::shared_ptr<Device>>& devices() const;

    const Cluster& cluster() const;
    void set_cluster(std::weak_ptr<Cluster> cluster);

    double bandwidth(const Device& from, const Device& to) const;
    double latency(const Device& from, const Device& to) const;

    static std::shared_ptr<Node> parse(std::string path);
};
}

