#include "team.h"

PatternTree::Team::Team(std::shared_ptr<PatternTree::Processor> processor, int cores)
{
    this->processor_ = processor;
    this->cores_ = cores;
};

int PatternTree::Team::cores() const
{
    return this->cores_;
};

const PatternTree::Processor& PatternTree::Team::processor() const
{
    return *(this->processor_);
};
