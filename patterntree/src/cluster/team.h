#pragma once

#include "cluster/processor.h"

namespace PatternTree
{
class Team {

int cores_;
std::shared_ptr<Processor> processor_;

public:
    Team(std::shared_ptr<Processor> processor, int cores);

    int cores() const;
    const PatternTree::Processor& processor() const;
};
}
