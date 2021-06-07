#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include <cwalk.h>

#include "cluster/node.h"
#include "cluster/processor.h"

using json = nlohmann::json;

namespace PatternTree
{
class Cluster {
std::string topology_;
std::vector<std::string> ordered_ids_;
std::vector<double> bandwidth_matrix_;
std::vector<double> latency_matrix_;
std::unordered_map<std::string, std::shared_ptr<Node>> nodes_;
std::unordered_map<std::string, std::string> addresses_;

public:
    Cluster(std::string topology,
        std::vector<std::string> ordered_ids,
        std::vector<double> bandwidth_matrix,
        std::vector<double> latency_matrix,
        std::unordered_map<std::string, std::shared_ptr<Node>> nodes,
        std::unordered_map<std::string, std::string> addresses
    );
   
    std::string topology() const;
    const std::unordered_map<std::string, std::shared_ptr<Node>>& nodes() const;

    double bandwidth(const Node& from, const Node& to) const;
    double latency(const Node& from, const Node& to) const;

    static std::shared_ptr<Cluster> parse(std::string path)
    {
        std::ifstream cluster_file(path);
        json cluster_json;
        cluster_file >> cluster_json;

        size_t base_path_end;
        cwk_path_get_dirname(path.c_str(), &base_path_end);
        std::string base_path = path.substr(0, base_path_end);

        std::vector<double> bandwidth_matrix;
        for (auto row : cluster_json["connectivity-bandwidth"]) {
            for (auto value : row) {
                bandwidth_matrix.push_back(value);
            }
        }

        std::vector<double> latency_matrix;
        for (auto row : cluster_json["connectivity-latency"]) {
            for (auto value : row) {
                latency_matrix.push_back(value);
            }
        }

        std::vector<std::string> ordered_ids;
        std::unordered_map<std::string, std::string> addresses;
        std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
        for (auto node_json : cluster_json["nodes"]) {
            char template_path[FILENAME_MAX];
            std::string template_path_relative = node_json["template"];
            cwk_path_join(base_path.c_str(), template_path_relative.c_str(), template_path, sizeof(template_path));

            std::shared_ptr<Node> node = Node::parse(template_path);
            std::string identifier = node_json["identifier"];
            std::string address = node_json["address"];
            nodes[identifier] = node;
            addresses[identifier] = address;

            ordered_ids.push_back(identifier);
        }

        std::shared_ptr<Cluster> cluster(new Cluster(
            cluster_json["topology"],
            ordered_ids,
            bandwidth_matrix,
            latency_matrix,
            nodes,
            addresses)
        );

        for (auto& entry : nodes)
        {
            entry.second->identifier_ = entry.first;
            entry.second->cluster_ = cluster;
        }
        return cluster;
    };

    enum Distance {
        PROCESSOR,
        DEVICE,
        NODE,
        CLUSTER
    };

    static Distance distance(const Processor& procA, const Processor& procB)
    {
        const Processor* pA = &procA;
        const Processor* pB = &procB;
        if (pA == pB)
        {
            return Distance::PROCESSOR;
        }

        auto deviceA = &(pA->device());
        auto deviceB = &(pB->device());
        if (deviceA == deviceB)
        {
            return Distance::DEVICE;
        }

        auto nodeA = &(deviceA->node());
        auto nodeB = &(deviceB->node());
        if (nodeA == nodeB)
        {
            return Distance::NODE;
        }

        return Distance::CLUSTER;
    };

    static const Processor& closest(const Processor& processor, std::vector<const Processor*>& processors)
    {
        int min_distance = 9999;
        const Processor* min_proc = 0;
        for (auto const& proc : processors)
        {
            auto dist = distance(processor, *proc);
            if (dist == Distance::PROCESSOR)
            {
                return *proc;
            }

            if (dist < min_distance)
            {
                min_distance = dist;
                min_proc = proc;
            }
        }
        return *min_proc;
    };

};
}
