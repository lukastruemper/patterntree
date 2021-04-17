#pragma once

#include "patterns/pattern.h"

namespace PatternTree {

class PatternSplit {
std::weak_ptr<IPattern> pattern_;
Dataflow subflow_in_;
Dataflow subflow_out_;
size_t begin_;
size_t end_;

public:
    PatternSplit(std::weak_ptr<IPattern> pattern, 
        Dataflow subflow_in,
        Dataflow subflow_out,
        size_t begin,
        size_t end
    );
    PatternSplit(std::weak_ptr<IPattern> pattern);

    IPattern& pattern() const;
    size_t begin() const;
    size_t end() const;
    int width() const;
    double flops() const;
	const Dataflow& consumes() const;
	const Dataflow& produces() const;

};

}
