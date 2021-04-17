#include "step.h"

PatternTree::Step::Step(std::vector<std::unique_ptr<PatternTree::IPattern>>& patterns, size_t index)
: index_(index), assignment_(), reverse_assigment_()
{
    for (auto it = patterns.begin(); it != patterns.end(); it++)
    {
            PatternTree::Step::add_pattern(std::move(*it));
    }
};

PatternTree::Step::Step(std::unique_ptr<PatternTree::IPattern> pattern, size_t index)
: index_(index), assignment_(), reverse_assigment_()
{
    PatternTree::Step::add_pattern(std::move(pattern));
};

void PatternTree::Step::add_pattern(std::unique_ptr<IPattern> pattern)
{
    std::shared_ptr<PatternTree::IPattern> p = std::move(pattern);
    this->patterns_.push_back(p);

    std::unique_ptr<PatternTree::PatternSplit> split(new PatternTree::PatternSplit(p));
    this->splits_.insert({p.get(), std::move(split)});
}

size_t PatternTree::Step::size() const
{
    return this->patterns_.size();
};

size_t PatternTree::Step::index() const
{
    return this->index_;
};

bool PatternTree::Step::complete() const
{
    return (this->splits_.size() == this->assignment_.size());
}

std::set<std::shared_ptr<PatternTree::Team>> PatternTree::Step::teams() const
{
    std::set<std::shared_ptr<Team>> teams;
    for (auto it = this->assignment_.begin(); it != this->assignment_.end(); ++it) {
        teams.insert(it->second);
    }
    return teams;
};

std::set<std::shared_ptr<PatternTree::Team>> PatternTree::Step::teams(const PatternTree::IPattern& pattern) const
{
    std::set<std::shared_ptr<Team>> teams;
    auto range = this->splits_.equal_range(&pattern);
    for (auto it = range.first; it != range.second; it++)
    {
        teams.insert(assignment_.at(it->second.get()));
    }

    return teams;
};

std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> PatternTree::Step::splits() const
{
    std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> splits;
    for (auto it = this->splits_.begin(); it != this->splits_.end(); it++)
    {
        splits.push_back(*(it->second));
    }

    return splits;
};

std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> PatternTree::Step::splits(const IPattern& pattern) const
{
    std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> splits;
    auto range = this->splits_.equal_range(&pattern);
    for (auto it = range.first; it != range.second; it++)
    {
        splits.push_back(*(it->second));
    }

    return splits;
};

std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> PatternTree::Step::split(const PatternTree::IPattern& pattern, std::vector<size_t> sizes)
{    
    this->free(pattern);
    this->splits_.erase(&pattern);

    auto pointer = *std::find_if(this->patterns_.begin(), this->patterns_.end(), [&pattern](std::shared_ptr<PatternTree::IPattern> p) {
		return p.get() == (&pattern);
	});

    std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> splits;
    size_t index = 0;
    size_t end = index + sizes[0];
    for (size_t i = 0; i < sizes.size(); i++) {   
        Dataflow subflow_in;
        for (auto& view : pointer->consumes()) {
            PatternTree::IData& data = *(view->data().lock());
            std::shared_ptr<PatternTree::IView> subview = PatternTree::IView::join(*(pointer->subflow_in(index, data)), *(pointer->subflow_in(end - 1, data)));
            subflow_in.push_back(subview);
        }

        Dataflow subflow_out;
        for (auto& view : pointer->produces()) {
            PatternTree::IData& data = *(view->data().lock());
            std::shared_ptr<PatternTree::IView> subview = PatternTree::IView::join(*(pointer->subflow_out(index)), *(pointer->subflow_out(end - 1)));
            subflow_out.push_back(subview);
        }
     
        std::unique_ptr<PatternTree::PatternSplit> split(new PatternTree::PatternSplit(
            pointer,
            subflow_in,
            subflow_out,
            index,
            end
        ));

        splits.push_back(*split);
        this->splits_.insert({pointer.get(), std::move(split)});

        if (i < sizes.size() - 1) {
            index = end;
            end = end + sizes[i + 1];
        }
    }

    return splits;
};

std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> PatternTree::Step::split(const PatternTree::IPattern& pattern, size_t n)
{
    std::vector<size_t> sizes(n, pattern.width() / n);
    sizes.push_back(pattern.width() % (pattern.width() / n));

    return this->split(pattern, sizes);
};

void PatternTree::Step::assign(const PatternTree::IPattern& pattern, std::shared_ptr<Team> team)
{
    auto range = this->splits_.equal_range(&pattern);
    for (auto it = range.first; it != range.second; it++) {
        PatternTree::Step::assign(*(it->second), team);
    }
};

void PatternTree::Step::assign(const PatternTree::PatternSplit& split, std::shared_ptr<Team> team)
{
    auto it = this->assignment_.find(&split);
    if (it != this->assignment_.end()) {
        this->assignment_.erase(it);

        auto reverse_range = this->reverse_assigment_.equal_range(it->second.get());
        for (auto reverse_it = reverse_range.first; reverse_it != reverse_range.second; reverse_it++) {
            if (reverse_it->second == it->first) {
                this->reverse_assigment_.erase(reverse_it);
                break;
            }
        }
    }
    this->assignment_.insert({&split, team});
    this->reverse_assigment_.insert({team.get(), &split});
};

void PatternTree::Step::free(const PatternTree::IPattern& pattern)
{
    auto range = this->splits_.equal_range(&pattern);
    for (auto it = range.first; it != range.second; it++) {
        PatternTree::Step::free(*(it->second));
    }
};

void PatternTree::Step::free(const PatternTree::PatternSplit& split)
{
    auto iter = this->assignment_.find(&split);
    if (iter == this->assignment_.end())
    {
        return;
    }

    this->assignment_.erase(iter);
    auto range = this->reverse_assigment_.equal_range(iter->second.get());
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == &split)
        {
            this->reverse_assigment_.erase(it);
            break;
        }
    }
};

std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> PatternTree::Step::assigned(const Team& team) const
{
    std::vector<std::reference_wrapper<const PatternTree::PatternSplit>> splits;
    auto range = this->reverse_assigment_.equal_range(&team);
    for (auto it = range.first; it != range.second; ++it) {
        splits.push_back(*(it->second));
    }
    return splits;
};

std::optional<std::shared_ptr<PatternTree::Team>> PatternTree::Step::assigned(const PatternTree::PatternSplit& split) const
{
    auto iter = this->assignment_.find(&split);
    if (iter == this->assignment_.end())
    {
        return std::nullopt;
    }

    return std::optional<std::shared_ptr<PatternTree::Team>>{ iter->second };
};

bool PatternTree::Step::happensBefore(const PatternTree::IPattern& pattern)
{
    for (auto it = this->begin(); it != this->end(); it++)
    {
        // TODO: Write after write
        for (auto consumed : pattern.consumes())
        {
            for (auto produced : it->produces())
            {
                bool disjoint = produced->disjoint(*consumed);
                if (!disjoint)
                {
                    return true;
                }
            }
        }
    }

    return false;
}