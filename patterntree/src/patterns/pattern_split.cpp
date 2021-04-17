#include "patterns/pattern_split.h"

PatternTree::PatternSplit::PatternSplit(
    std::weak_ptr<IPattern> pattern, 
    Dataflow subflow_in,
    Dataflow subflow_out,
    size_t begin,
    size_t end
) : pattern_(pattern), subflow_in_(subflow_in), subflow_out_(subflow_out), begin_(begin), end_(end)
{};

PatternTree::PatternSplit::PatternSplit(std::weak_ptr<IPattern> pattern)
{
    this->pattern_ = pattern;

    auto p = pattern.lock();
    this->subflow_in_ = p->consumes();
    this->subflow_out_ = p->produces();
    this->begin_ = 0;
    this->end_ = p->width();

    // TODO: Convex index optimization
};

PatternTree::IPattern& PatternTree::PatternSplit::pattern() const
{
    auto pattern = this->pattern_.lock();
    return *pattern;
}

size_t PatternTree::PatternSplit::begin() const
{
    return this->begin_;
};

size_t PatternTree::PatternSplit::end() const
{
    return this->end_;
};

int PatternTree::PatternSplit::width() const
{
    return this->end_ - this->begin_;
};

double PatternTree::PatternSplit::flops() const
{
    auto pattern = this->pattern_.lock();
    double lb = pattern->flops(this->begin_, true);
    double ub = pattern->flops(this->end_ - 1, true);

    return (lb + ub) / 2.0 * this->width();
};

const PatternTree::Dataflow& PatternTree::PatternSplit::consumes() const
{
    return this->subflow_in_;
};

const PatternTree::Dataflow& PatternTree::PatternSplit::produces() const
{
    return this->subflow_out_;
};
