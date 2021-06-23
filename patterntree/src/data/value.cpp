#include "value.h"


PatternTree::IValue::IValue(std::string identifier, size_t dim0, size_t dim1)
: identifier_(identifier)
{
	if (dim1 == 0){
		this->shape_ = std::vector<size_t>{ dim0 };
	} else {
		this->shape_ = std::vector<size_t>{ dim0, dim1 };
	}
}

std::string PatternTree::IValue::identifier() const
{
	return this->identifier_;
}

std::vector<size_t> PatternTree::IValue::shape() const
{
	return this->shape_;
}
