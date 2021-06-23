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
    friend class Device;

    Processor(int cores, int arithmetic_units, double frequency, double cache_size, double cache_latency, double cache_bandwidth);
    
    /**
     * Number of cores.
     * 
     * @return cores
     */
    int cores() const;
    
    /**
     * Number of arithmetic units.
     * 
     * @return arithmetic units
     */
    int arithmetic_units() const;

    /**
     * Clock frequency.
     * 
     * @return frequency
     */
    double frequency() const;

    /**
     * Cache size in MB.
     * 
     * @return cache size
     */
    double cache_size() const;

    /**
     * Cache latency in nanoseconds.
     * 
     * @return latency
     */
    double cache_latency() const;

    /**
     * Cache bandwidth in MB/s.
     * 
     * @return cache bandwidth
     */
    double cache_bandwidth() const;

    /**
     * Device, on which the processor is located.
     * 
     * @return device
     */
    const Device& device() const;

    /**
     * Generates json summary of processor.
     * Note this is not the original cluster description.
     * 
     * @return nlohmann::json
     */
    json to_json() const;

    static std::shared_ptr<Processor> parse(std::string path);

};
}
