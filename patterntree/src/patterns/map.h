#pragma once

#include <functional>
#include <unordered_map>
#include <utility>
#include <type_traits>


#include <Kokkos_Core.hpp>

#include "patterns/pattern.h"
#include "data/data.h"
#include "data/view.h"

namespace PatternTree
{

template<typename D>
struct MapFunctor {

	virtual void operator () (const int index, View<D>& element) = 0;
	virtual void consumes(Dataflow& dataflow) = 0;

	virtual bool touch(const int index, PatternIndexInfo &info) { return false; };
};

template<typename T, typename D>
concept MAPFUNCTOR = (std::is_base_of<MapFunctor<D>, T>::value);

template<typename D>
class Map : public IPattern {

std::unique_ptr<MapFunctor<D>> func_;
std::shared_ptr<View<D>> field_;

template<typename Functor>
static Dataflow in_data(Functor& functor, std::shared_ptr<IView> field)
{
	Dataflow data;
	data.push_back(field);
	functor.consumes(data);

	return data;
};

static Dataflow out_data(std::shared_ptr<IView> field)
{
	Dataflow data;
	data.push_back(field);

	return data;
};

public:
	Map(std::unique_ptr<MapFunctor<D>> func, std::shared_ptr<View<D>> field, Dataflow in_data, Dataflow out_data)
	: 	IPattern(in_data, out_data, field->shape().at(0)),
		func_(std::move(func)),
		field_(field)
	{};

	std::shared_ptr<PatternTree::IView> subflow_out(const int index) override {
		return View<D>::element(this->field_->data(), index);
	};

	void touch(const int index) override
	{
			// Option A: Call overriden touch function
			PatternIndexInfo stats;
			stats.index = index;
			if (this->func_->touch(index, stats)) {
				this->info_[index] = stats;
				return;
			}

			// Option B: Execute actual function
			// - counts flops
			// - TODO: constructs subviews

			std::shared_ptr<Data<D>> data = std::static_pointer_cast<Data<D>>(this->field_->data().lock());
			std::shared_ptr<View<D>> element = View<D>::element(data, index);

			element->set_nested_context(true);
			element->reset_FLOPS();
			func_.get()->operator()(index, *element);
			element->set_nested_context(false);

			stats.flops = element->reset_FLOPS();
			this->info_[index] = stats;
	};
	
	template<typename Functor>
	static std::unique_ptr<Map<D>> create(std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field, size_t interpolation_frequency) requires MAPFUNCTOR<Functor, D>
	{
		Dataflow in_flow = in_data(*functor, field);
		Dataflow out_flow = out_data(field);
		std::unique_ptr<Map<D>> map(new Map<D>(std::move(functor), field, in_flow, out_flow));

		// Gather info
		std::vector<int> shape = field->shape();
		size_t interpolation_width = std::max(shape[0] / interpolation_frequency, (size_t) 1);
		for (size_t i = 0; i < shape[0]; i = i + interpolation_width)
		{
			map->touch(i);
		}
		map->touch(shape[0] - 1);

		return map;
	};

};

}
