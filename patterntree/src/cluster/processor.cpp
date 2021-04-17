#include "processor.h"

PatternTree::Processor::Processor(int cores, int arithmetic_units, double frequency,
    double cache_size, double cache_latency, double cache_bandwidth)
{
    this->cores_ = cores;
    this->arithmetic_units_ = arithmetic_units;
    this->frequency_ = frequency;

    this->cache_size_ = cache_size;
    this->cache_latency_ = cache_latency;
    this->cache_bandwidth_ = cache_bandwidth;
};

int PatternTree::Processor::cores() const
{
    return this->cores_;
};

int PatternTree::Processor::arithmetic_units() const
{
    return this->arithmetic_units_;
};

double PatternTree::Processor::frequency() const
{
    return this->frequency_;
};

double PatternTree::Processor::cache_size() const
{
    return this->cache_size_;
};

double PatternTree::Processor::cache_latency() const
{
    return this->cache_latency_;
};

double PatternTree::Processor::cache_bandwidth() const
{
    return this->cache_bandwidth_;
};

void PatternTree::Processor::set_device(std::weak_ptr<PatternTree::Device> device)
{
    this->device_ = device;
};

const PatternTree::Device& PatternTree::Processor::device() const
{
    return *(this->device_.lock());
}

std::shared_ptr<PatternTree::Processor> PatternTree::Processor::parse(std::string path)
{
    std::ifstream processor_file(path);
    json processor_json;
    processor_file >> processor_json;

    auto highest_cache_json = processor_json["caches"].back();
    std::shared_ptr<PatternTree::Processor> processor(new PatternTree::Processor(
        processor_json["cores"],
        processor_json["arithmetic-units"],
        processor_json["frequency"],
        highest_cache_json["size"],
        highest_cache_json["latency"],
        highest_cache_json["bandwidth"]
    ));

    return processor;
};