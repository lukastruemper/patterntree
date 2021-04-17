#pragma once

#include <apt/apt.h>
#include <apt/step.h>
#include <cluster/cluster.h>

namespace PatternTree
{

class IOptimizer {
    public:
        virtual void init(APT::Iterator begin, APT::Iterator end, const Cluster& cluster) = 0;
        virtual void assign(APT::Iterator& step) = 0;
        virtual double costs() = 0;
};

}
