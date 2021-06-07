#include "device.h"

PatternTree::Device::Device(std::string type, double memory_size, double memory_latency, double memory_bandwidth, double memory_max_bandwidth, std::unordered_map<std::string, std::shared_ptr<PatternTree::Processor>> processors)
: identifier_(""), type_(type), memory_size_(memory_size), memory_latency_(memory_latency), memory_bandwidth_(memory_bandwidth), memory_max_bandwidth_(memory_max_bandwidth), processors_(processors)
{};

std::string PatternTree::Device::identifier() const
{
    return this->identifier_;
};

std::string PatternTree::Device::type() const
{
    return this->type_;
};

double PatternTree::Device::memory_size() const
{
    return this->memory_size_;
};

double PatternTree::Device::memory_latency() const
{
    return this->memory_latency_;
};

double PatternTree::Device::memory_bandwidth() const
{
    return this->memory_bandwidth_;
};

double PatternTree::Device::memory_max_bandwidth() const
{
    return this->memory_max_bandwidth_;
};

const std::unordered_map<std::string, std::shared_ptr<PatternTree::Processor>>& PatternTree::Device::processors() const
{
    return this->processors_;
};

const PatternTree::Node& PatternTree::Device::node() const
{
    return *(this->node_.lock());
}

std::shared_ptr<PatternTree::Device> PatternTree::Device::parse(std::string path)
{
    std::ifstream device_file(path);
    json device_json;
    device_file >> device_json;

    size_t base_path_end;
    cwk_path_get_dirname(path.c_str(), &base_path_end);
    std::string base_path = path.substr(0, base_path_end);

    std::unordered_map<std::string, std::shared_ptr<PatternTree::Processor>> processors;
    for (auto processor_json : device_json["cache-group"])
    {
        char template_path[FILENAME_MAX];
        std::string template_path_relative = processor_json["template"];
        cwk_path_join(base_path.c_str(), template_path_relative.c_str(), template_path, sizeof(template_path));

        std::shared_ptr<PatternTree::Processor> processor = PatternTree::Processor::parse(template_path);
        std::string identifier = processor_json["identifier"];
        processors[identifier] = processor;
    }

    std::shared_ptr<PatternTree::Device> device(new PatternTree::Device(
        device_json["type"],
        device_json["size"],
        device_json["latency"],
        device_json["bandwidth"],
        device_json["max-bandwidth"],
        processors
    ));

    for (auto const& processor : processors)
    {
        processor.second->device_ = device;
    }

    return device;
};
