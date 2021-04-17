#include <numeric>

#include "view.h"

std::weak_ptr<PatternTree::IData> PatternTree::IView::data()
{
	return this->data_;
};

bool PatternTree::IView::is_symbolic() const
{
	return this->data_.lock()->is_symbolic();
};

std::string PatternTree::IView::name() const
{
	return this->data_.lock()->name();
};

std::vector<int> PatternTree::IView::shape() const
{
	return this->shape_;
};

std::vector<int> PatternTree::IView::begins() const
{
	return this->begins_;
};

std::vector<int> PatternTree::IView::ends() const
{
	return this->ends_;
};

int PatternTree::IView::elements() const
{
	int elements = std::accumulate(std::begin(this->shape_), std::end(this->shape_), 1, std::multiplies<int>());
	return elements;
}

bool PatternTree::IView::is_nested_context() const
{
	return this->nested_context_;
};

double PatternTree::IView::kbytes() const
{
	return (8.0 * this->elements()) / 1000.0;
}

void PatternTree::IView::set_nested_context(bool nested)
{
	this->nested_context_ = nested;
};

void PatternTree::IView::add_FLOPS(int flops)
{
	this->flops_ += flops;
};

int PatternTree::IView::reset_FLOPS()
{
	int temp = this->flops_;
	this->flops_ = 0;

	return temp;
};
