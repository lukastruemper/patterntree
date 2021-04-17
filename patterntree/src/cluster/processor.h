#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

using json = nlohmann::json;


namespace PatternTree
{

class Device;

class Processor {
int cores_;
int arithmetic_units_;
double frequency_;

double cache_size_;
double cache_latency_;
double cache_bandwidth_;

std::weak_ptr<Device> device_;

public:
    Processor(int cores, int arithmetic_units, double frequency, double cache_size, double cache_latency, double cache_bandwidth);
    
    int cores() const;
    int arithmetic_units() const;
    double frequency() const;
    double cache_size() const;
    double cache_latency() const;
    double cache_bandwidth() const;

    const Device& device() const;
    void set_device(std::weak_ptr<Device> device);


    static std::shared_ptr<Processor> parse(std::string path);

};
}
