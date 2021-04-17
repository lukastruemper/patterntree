#include "node.h"

PatternTree::Node::Node(std::string type, std::unordered_map<std::string, std::shared_ptr<Device>> devices)
{
    this->type_ = type;
    this->devices_ = devices;
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

void PatternTree::Node::set_cluster(std::weak_ptr<PatternTree::Cluster> cluster)
{
    this->cluster_ = cluster;
}

double PatternTree::Node::bandwidth(const PatternTree::Device& from, const PatternTree::Device& to) const
{
    return 12152.0;
};

double PatternTree::Node::latency(const PatternTree::Device& from, const PatternTree::Device& to) const
{
    return 7210.0;
};

std::shared_ptr<PatternTree::Node> PatternTree::Node::parse(std::string path)
{
    std::ifstream node_file(path);
    json node_json;
    node_file >> node_json;

    size_t base_path_end;
    cwk_path_get_dirname(path.c_str(), &base_path_end);
    std::string base_path = path.substr(0, base_path_end);

    std::unordered_map<std::string, std::shared_ptr<PatternTree::Device>> devices;
    for (auto device_json : node_json["devices"])
    {
        char template_path[FILENAME_MAX];
        std::string template_path_relative = device_json["template"];
        cwk_path_join(base_path.c_str(), template_path_relative.c_str(), template_path, sizeof(template_path));

        std::string identifier = device_json["identifier"];
        std::shared_ptr<PatternTree::Device> device = PatternTree::Device::parse(template_path);
        devices[identifier] = device;
    }

    std::shared_ptr<PatternTree::Node> node(new PatternTree::Node(node_json["type"], devices));
    for (auto const& device : devices)
    {
        device.second->set_node(node);
    }
    return node;
};