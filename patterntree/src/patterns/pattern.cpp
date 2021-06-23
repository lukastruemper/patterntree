#include "pattern.h"

PatternTree::IPattern::IPattern(std::string identifier, PatternTree::Dataflow data_in, PatternTree::Dataflow data_out, int width)
: 	flow_in_(data_in),
	flow_out_(data_out),
	width_(width),
	identifier_(identifier),
	info_()
{};

PatternTree::Dataflow PatternTree::IPattern::consumes() const
{
	return this->flow_in_;
};

PatternTree::Dataflow PatternTree::IPattern::produces() const
{
	return this->flow_out_;
};

std::string PatternTree::IPattern::identifier() const
{
	return this->identifier_;
}

int PatternTree::IPattern::width() const
{
	return this->width_;
};

double PatternTree::IPattern::flops() const
{
	double flops_mean = 0.0;
	for (auto it = info_.begin(); it != info_.end(); it++)
	{
		flops_mean += it->second.flops;
	}
    flops_mean /= this->info_.size();
	return flops_mean * this->width_;
};

double PatternTree::IPattern::flops(const int index, bool touch)
{
	auto it = this->info_.find(index);
	if (it != this->info_.end()) {
		return it->second.flops;
	}

	if (!touch) {
		auto lb = this->info_.lower_bound(index);
		auto ub = this->info_.upper_bound(index);

		if (lb != this->info_.begin()) {
			lb--;
		}

		// Linear interpolation
		double m = (ub->second.flops - lb->second.flops) / (ub->first - lb->first);
		return m * (index - lb->first) + lb->second.flops;
	}

	this->touch(index);
	it = this->info_.find(index);
	return it->second.flops;
};

std::shared_ptr<PatternTree::IView> PatternTree::IPattern::subflow_in(const int index, PatternTree::IData& data)
{
	auto it = this->info_.find(index);
	if (it == this->info_.end()) {
		this->touch(index);
		it = this->info_.find(index);
	}

	auto subview = it->second.subviews.find(&data);
	if (subview == it->second.subviews.end()) {
		auto superview = std::find_if(this->flow_in_.begin(), this->flow_in_.end(), [&data](std::shared_ptr<IView> view) {
			auto d = view->data().lock();
			return d.get() == (&data);
		});
		return *superview;
	}

	return subview->second;
};
