#pragma once

#include <numeric>
#include <memory>

#include "apt/apt.h"
#include "data/view.h"
#include "patterns/map.h"

namespace PatternTree {


template<typename D>
struct IncrementFunctor : public MapFunctor<D> {
	void operator () (int index, View<D>& element) override {
		return;
	};

	void consumes(PatternTree::Dataflow& dataflow) override {};
};

namespace Arithmetic
{


	template<typename T>
	void increment(std::shared_ptr<View<T>> view)
	{
		if (view->is_symbolic())
		{
			if (view->is_nested_context())
			{
				int flops = view->elements() * 1;
				view->add_FLOPS(flops);
			}
			else {
				std::vector<int> shape = view->shape();
				int inner_dim = std::accumulate(std::begin(shape) + 1, std::end(shape), 1, std::multiplies<int>());
				
				std::unordered_map<int, int> flops = {
					{0, inner_dim * 1},
					{view->shape().at(0) - 1, inner_dim * 1}
				};

				std::unique_ptr<IncrementFunctor<T>> functor(new IncrementFunctor<T>());
				APT::map<T, IncrementFunctor<T>>(std::move(functor), view);
			}
		} else
		{
			std::cout << "Not implemented" << std::endl;
		}
	};

	template<typename T>
	void increment(View<T>& view)
	{
		if (view.is_symbolic())
		{
			if (view.is_nested_context())
			{
				int flops = view.elements() * 1;
				view.add_FLOPS(flops);
			}
		}
	};
}

}
