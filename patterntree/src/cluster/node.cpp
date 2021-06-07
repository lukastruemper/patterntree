#include "node.h"

PatternTree::Node::Node(std::string type,
    std::vector<std::string> ordered_ids,
    std::vector<double> bandwidth_matrix,
    std::vector<double> latency_matrix,
    std::unordered_map<std::string, std::shared_ptr<Device>> devices
)
: identifier_(""), type_(type), ordered_ids_(ordered_ids), bandwidth_matrix_(bandwidth_matrix), latency_matrix_(latency_matrix), devices_(devices)
{};

std::string PatternTree::Node::identifier() const
{
    return this->identifier_;;
};

std::string PatternTree::Node::type() const
{
    return this->type_;
};

const std::unordered_map<std::string, std::shared_ptr<PatternTree::Device>>& PatternTree::Node::devices() const
{
    return this->devices_;
};

const PatternTree::Cluster& PatternTree::Node::cluster() const
{
    return *(this->cluster_.lock());
};

double PatternTree::Node::bandwidth(const PatternTree::Device& from, const PatternTree::Device& to) const
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

double PatternTree::Node::latency(const PatternTree::Device& from, const PatternTree::Device& to) const
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

std::shared_ptr<PatternTree::Node> PatternTree::Node::parse(std::string path)
{
    std::ifstream node_file(path);
    json node_json;
    node_file >> node_json;

    size_t base_path_end;
    cwk_path_get_dirname(path.c_str(), &base_path_end);
    std::string base_path = path.substr(0, base_path_end);

    std::vector<double> bandwidth_matrix;
    for (auto row : node_json["connectivity-bandwidth"]) {
        for (auto value : row) {
            bandwidth_matrix.push_back(value);
        }
    }

    std::vector<double> latency_matrix;
    for (auto row : node_json["connectivity-latency"]) {
        for (auto value : row) {
            latency_matrix.push_back(value);
        }
    }

    std::vector<std::string> ordered_ids;
    std::unordered_map<std::string, std::shared_ptr<PatternTree::Device>> devices;
    for (auto device_json : node_json["devices"])
    {
        char template_path[FILENAME_MAX];
        std::string template_path_relative = device_json["template"];
        cwk_path_join(base_path.c_str(), template_path_relative.c_str(), template_path, sizeof(template_path));

        std::string identifier = device_json["identifier"];
        std::shared_ptr<PatternTree::Device> device = PatternTree::Device::parse(template_path);
        devices[identifier] = device;

        ordered_ids.push_back(identifier);
    }

    std::shared_ptr<PatternTree::Node> node(new PatternTree::Node(
        node_json["type"],
        ordered_ids,
        bandwidth_matrix,
        latency_matrix,
        devices)
    );

    for (auto const& device : devices)
    {
        device.second->identifier_ = device.first;
        device.second->node_ = node;
    }
    return node;
};