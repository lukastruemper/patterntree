#include "view.h"

#include <numeric>

PatternTree::IView::IView(std::weak_ptr<IData> data, std::pair<size_t, size_t> dim0, std::pair<size_t, size_t> dim1)
: data_(data), nested_(false)
{
	if (dim1.first < dim1.second) {
		this->begins_ = std::vector<size_t>{ dim0.first, dim1.first };
		this->ends_ = std::vector<size_t>{ dim0.second, dim1.second };
		this->shape_ = std::vector<size_t>{ dim0.second - dim0.first, dim1.second - dim1.first };
	} else {
		this->begins_ = std::vector<size_t>{ dim0.first };
		this->ends_ = std::vector<size_t>{ dim0.second };
		this->shape_ = std::vector<size_t>{ dim0.second - dim0.first };
	}
}

std::weak_ptr<PatternTree::IData> PatternTree::IView::data()
{
	return this->data_;
}

std::vector<size_t> PatternTree::IView::shape() const
{
	return this->shape_;
}

std::vector<size_t> PatternTree::IView::begins() const
{
	return this->begins_;
}

std::vector<size_t> PatternTree::IView::ends() const
{
	return this->ends_;
}

size_t PatternTree::IView::elements() const
{
	size_t elements = std::accumulate(std::begin(this->shape_), std::end(this->shape_), 1, std::multiplies<size_t>());
	return elements;
}


double PatternTree::IView::kbytes() const
{
	return (8.0 * this->elements()) / 1000.0;
}

std::shared_ptr<PatternTree::IView> PatternTree::IView::join(PatternTree::IView& view1, PatternTree::IView& view2)
{
	if (view1.data_.lock() != view2.data_.lock()) {
		// error
		return nullptr;
	}

	size_t begin_0 = std::min(view1.begins_[0], view2.begins_[0]);
	size_t end_0 = std::max(view1.ends_[0], view2.ends_[0]);

	std::shared_ptr<IView> joined = view1.clone();
	joined->begins_ = { begin_0 };
	joined->ends_ = { end_0 };
	joined->shape_ = { end_0 - begin_0 };

	if (view1.shape_.size() == 2) {
		size_t end_1 = std::max(view1.ends_[1], view2.ends_[1]);
		size_t begin_1 = std::min(view1.begins_[1], view2.begins_[1]);

		joined->begins_.push_back(begin_1);
		joined->ends_.push_back(end_1);
		joined->shape_.push_back(end_1 - begin_1);
	}

	return joined;
}

std::unordered_set<std::shared_ptr<PatternTree::IView>> PatternTree::IView::as_basis(PatternTree::IView& view)
{
	auto data = view.data_.lock();
	std::unordered_set<std::shared_ptr<IView>> view_basis;

	std::vector<size_t> begins = view.begins();
	std::vector<size_t> ends = view.ends();

	size_t split_0 = data->split_size()[0];
	size_t start_0 = floor(begins[0] / (float) split_0);
	size_t end_0 = ceil(ends[0] / (float) split_0);
	for (size_t i = start_0; i < end_0; i++)
	{
		if (begins.size() == 1)
		{
			view_basis.insert(data->basis().at(i));
		} else {
			size_t split_1 = data->split_size()[1];
			size_t start_1 = floor(begins[1] / (float) split_1);
			size_t end_1 = ceil(ends[1] / (float) split_1);

			size_t length_1 = ceil(data->shape()[1] / (float) split_1);
			for (size_t j = start_1; j < end_1; j++)
			{
				view_basis.insert(data->basis().at(i * length_1 + j));
			}
		}
	}

	return view_basis;
};
