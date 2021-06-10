#include "dataflow_state.h"

PatternTree::DataflowState::DataflowState()
: index_(0), table_(), reverse_table_()
{};

size_t PatternTree::DataflowState::index() const
{
    return this->index_;
};

void PatternTree::DataflowState::reads(const PatternTree::Processor& processor, PatternTree::IView& view)
{
    for (auto const& basis_view : IView::as_basis(view))
    {
        this->table_.insert({basis_view, &processor});
        this->reverse_table_.insert({&processor, basis_view});
    }
};

void PatternTree::DataflowState::writes(const PatternTree::Processor& processor, PatternTree::IView& view)
{
    for (auto const& basis_view : IView::as_basis(view))
    {
        auto range = this->table_.equal_range(basis_view);
        for (auto it = range.first; it != range.second;) {
            const PatternTree::Processor* temp = it->second;
            auto team_range = this->reverse_table_.equal_range(temp);
            for (auto team_it = team_range.first; team_it != team_range.second; ++team_it) {
                if (team_it->second == basis_view)
                {
                    this->reverse_table_.erase(team_it);
                    break;
                }
            }

            it = this->table_.erase(it);
        }

        this->table_.insert({basis_view, &processor});
        this->reverse_table_.insert({&processor, basis_view});
    }
};

std::unordered_multimap<std::shared_ptr<PatternTree::IView>, const PatternTree::Processor*> PatternTree::DataflowState::owned_by(PatternTree::IView& view) const
{
    std::unordered_multimap<std::shared_ptr<PatternTree::IView>, const PatternTree::Processor*> map;
    for (auto const& basis_view : IView::as_basis(view))
    {
        auto range = this->table_.equal_range(basis_view);
        for (auto it = range.first; it != range.second; ++it) {
            map.insert({basis_view, it->second});
        }
    }

    return map;
};

std::set<std::shared_ptr<PatternTree::IView>> PatternTree::DataflowState::owns(const PatternTree::Processor& processor) const
{
    std::set<std::shared_ptr<PatternTree::IView>> views;
    auto range = this->reverse_table_.equal_range(&processor);
    for (auto it = range.first; it != range.second; ++it) {
        views.insert(it->second);
    }
    return views;
};

void PatternTree::DataflowState::update(PatternTree::Step& step)
{
    if (step.index() != this->index_ || !step.complete())
    {
        //std::cout << "Inconsistent state update" << std::endl;
        return;
    }

    for (auto it = step.begin(); it != step.end(); ++it)
    {
        PatternTree::IPattern& pattern = *it;
        for (std::reference_wrapper<const PatternTree::PatternSplit>& split_ :  step.splits(pattern)) {
            const PatternTree::PatternSplit& split = split_.get();
            
            const PatternTree::Processor& processor = step.assigned(split).value()->processor();

            for (auto const& consumed : split.consumes())
            {
                this->reads(processor, *consumed);
            }
            for (auto const& produced : split.produces())
            {
                this->writes(processor, *produced);
            }
        }
    }

    this->index_++;
};
