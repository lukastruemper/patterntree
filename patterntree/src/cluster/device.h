#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <cwalk.h>

#include "cluster/processor.h"

using json = nlohmann::json;

namespace PatternTree 
{

class Node;

class Device {
std::string type_;
double memory_size_;
double memory_latency_;
double memory_bandwidth_;
double memory_max_bandwidth_;

std::weak_ptr<Node> node_;

std::unordered_map<std::string, std::shared_ptr<Processor>> processors_;

public:
    Device(std::string type, double memory_size, double memory_latency, double memory_bandwidth, double memory_max_bandwidth, std::unordered_map<std::string, std::shared_ptr<Processor>> processors);

    std::string type() const;
    double memory_size() const;
    double memory_latency() const;
    double memory_bandwidth() const;
    double memory_max_bandwidth() const;
    const std::unordered_map<std::string, std::shared_ptr<Processor>>& processors() const;

    const Node& node() const;
    void set_node(std::weak_ptr<Node> node);

    static std::shared_ptr<Device> parse(std::string);
};
}
