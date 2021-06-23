#include "data.h"

#include "data/view.h"

PatternTree::IData::IData(std::unique_ptr<PatternTree::IValue> value)
: value_(std::move(value)), basis_(), split_size_()
{};

PatternTree::IValue& PatternTree::IData::value() const
{
	return *(this->value_);
}

std::string PatternTree::IData::identifier() const
{
	return this->value_->identifier();
}

std::vector<size_t> PatternTree::IData::shape() const
{
	return this->value_->shape();
}

std::vector<std::shared_ptr<PatternTree::IView>> PatternTree::IData::basis() const
{
    return this->basis_;
}

std::vector<size_t> PatternTree::IData::split_size() const
{
	return this->split_size_;
}
