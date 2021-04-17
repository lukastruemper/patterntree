#pragma once

#include <unordered_map>
#include <set>

#include "data/view.h"
#include "apt/step.h"
#include "cluster/processor.h"

namespace PatternTree
{
class DataflowState {

size_t index_;
std::unordered_multimap<std::shared_ptr<IView>, const Processor*> table_;
std::unordered_multimap<const Processor*, std::shared_ptr<IView>> reverse_table_;

void reads(const Processor& processor, IView& view);
void writes(const Processor& processor, IView& view);

public:
    DataflowState();

    size_t index() const;
    std::unordered_multimap<std::shared_ptr<IView>, const Processor*> owned_by(IView& view) const;
    std::set<std::shared_ptr<IView>> owns(const Processor& processor) const;

    void update(Step& step);
};

}
