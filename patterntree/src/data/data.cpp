#include "data.h"

#include "data/view.h"

PatternTree::IData::IData(std::string name, int dim0, int dim1)
: name_(name), symbolic_(true)
{
	std::vector<int> shape;
	shape.push_back(dim0);
	if (dim1 > 0)
	{
		shape.push_back(dim1);
	}

	this->shape_ = shape;
};

std::string PatternTree::IData::name() const
{
	return this->name_;
};

std::vector<int> PatternTree::IData::shape() const
{
	return this->shape_;
};

bool PatternTree::IData::is_symbolic() const
{
	return this->symbolic_;
};
