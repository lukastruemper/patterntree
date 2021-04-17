#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <math.h> 

#include <iterator>
#include <cstddef> 

#include "apt/step.h"
#include "data/data.h"
#include "data/view.h"
#include "patterns/map.h"
#include "patterns/pattern.h"
#include "performance/performance_model.h"

namespace PatternTree {

class Cluster;
class IOptimizer;

class APT {
static APT* instance;
APT(std::shared_ptr<Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency, bool synchronization_efficiency)
: 	cluster_(cluster),
	operation_interpolation_frequency_(operation_interpolation_frequency),
	data_interpolation_frequency_(data_interpolation_frequency),
	synchronization_efficiency_(synchronization_efficiency),
	synchronization_efficiency_length_(-1)
{};

bool synchronization_efficiency_;
int synchronization_efficiency_length_;
size_t operation_interpolation_frequency_;
size_t data_interpolation_frequency_;
std::shared_ptr<Cluster> cluster_;
std::vector<std::shared_ptr<IData>> sources_;
std::vector<std::unique_ptr<Step>> flow_;

public:
	
	struct Iterator 
	{
		using iterator_category = std::input_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using value_type        = Step;
		using pointer           = Step*;
		using reference         = Step&;

		Iterator(std::vector<std::unique_ptr<Step>>::iterator iter)
		: iter_(iter)
		{}

		reference operator*() const { return *(iter_->get()); }
		pointer operator->() { return iter_->get(); }

		Iterator& operator++()
		{ 
			iter_++;
			return *this;
		}  

		Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

		friend bool operator== (const Iterator& a, const Iterator& b) { return a.iter_ == b.iter_; };
		friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iter_ != b.iter_; };

		private:
			std::vector<std::unique_ptr<Step>>::iterator iter_;
	};

	size_t size();
	const Cluster& cluster();
	const std::vector<std::shared_ptr<IData>>& sources();
	
	APT::Iterator begin() { return Iterator( this->flow_.begin() ); }
    APT::Iterator end()   { return Iterator( this->flow_.end() ); }

	void optimize(IOptimizer& optimizer);
	double evaluate(IPerformanceModel& model);
	void summary();

	static void initialize(std::shared_ptr<Cluster> cluster);
	static void initialize(std::shared_ptr<Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency);
	static void initialize(std::shared_ptr<Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency, bool synchronization_efficiency);

	static void operation_interpolation_frequency(size_t frequency);
	static void data_interpolation_frequency(size_t frequency);
	static void synchronization_efficiency(bool enabled);
	static void synchronization_efficiency_length(int length);

	static std::unique_ptr<APT> compile();

	template<typename D>
	static std::shared_ptr<View<D>> source(std::string name, int dim0) requires ONEDIM<D>
	{
		size_t frequency = APT::instance->data_interpolation_frequency_;
		return APT::source<D>(name, dim0, frequency);
	}

	template<typename D>
	static std::shared_ptr<View<D>> source(std::string name, int dim0, size_t interpolation_frequency) requires ONEDIM<D>
	{

		std::shared_ptr<Data<D>> data(new Data<D>(name, dim0));
		size_t basis_split_0_ = std::max(dim0 / interpolation_frequency, interpolation_frequency);
		data->split_size_ = { basis_split_0_ };
		for (size_t i = 0; i < dim0; i += basis_split_0_)
		{
			size_t length_0 = std::min(basis_split_0_, dim0 - i);

			std::shared_ptr<View<D>> basis_view = View<D>::slice(data, std::make_pair(i, i + length_0));
			data->basis_.push_back(basis_view);
		}
		APT::instance->sources_.push_back(data);

		auto view = View<D>::full(data);
		return view;
	};

	template<typename D>
	static std::shared_ptr<View<D>> source(std::string name, int dim0, int dim1) requires TWODIM<D>
	{
		size_t frequency = APT::instance->data_interpolation_frequency_;
		return APT::source<D>(name, dim0, dim1, frequency);
	}

	template<typename D>
	static std::shared_ptr<View<D>> source(std::string name, int dim0, int dim1, size_t interpolation_frequency) requires TWODIM<D>
	{
		std::shared_ptr<Data<D>> data(new Data<D>(name, dim0, dim1));
		size_t basis_split_0_ = std::max(dim0 / interpolation_frequency, interpolation_frequency);
		size_t basis_split_1_ = std::max(dim1 / interpolation_frequency, interpolation_frequency);
		data->split_size_ = { basis_split_0_, basis_split_1_ };
		for (size_t i = 0; i < dim0; i += basis_split_0_)
		{
			size_t length_0 = std::min(basis_split_0_, dim0 - i);
			for (size_t j = 0; j < dim1; j += basis_split_1_)
			{
				size_t length_1 = std::min(basis_split_1_, dim1 - j);

				std::shared_ptr<View<D>> basis_view = View<D>::slice(data, std::make_pair(i, i + length_0), std::make_pair(j, j + length_1));
				data->basis_.push_back(basis_view);
			}
		}
		APT::instance->sources_.push_back(data);

		auto view = View<D>::full(data);
		return view;
	};

	template<typename D, typename Functor>
	static void map(std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field) requires MAPFUNCTOR<Functor, D>
	{
		size_t frequency = instance->operation_interpolation_frequency_;
		map(std::move(functor), field, frequency);
	}

	template<typename D, typename Functor>
	static void map(std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field, size_t interpolation_fequency) requires MAPFUNCTOR<Functor, D>
	{
		auto map = Map<D>::create(std::move(functor), field, interpolation_fequency);
		
		if (instance->flow_.size() == 0 || !instance->synchronization_efficiency_)
		{
			std::unique_ptr<Step> step(new Step(std::move(map), instance->flow_.size()));
			instance->flow_.push_back(std::move(step));

			return;
		}

		auto prev_ = instance->flow_.end() - 1;

		int history_length = std::min(instance->synchronization_efficiency_length_, (int) instance->flow_.size());
		if (history_length < 0) {
			history_length = instance->flow_.size();
		}
		
		for (int i = 0; i < history_length; i++) {
			auto step_ = prev_->get();
			if (step_->happensBefore(*map)) {
				if (i == 0) {
					std::unique_ptr<Step> step(new Step(std::move(map), instance->flow_.size()));
					instance->flow_.push_back(std::move(step));
					return;
				} else {
					break;
				}
			}

			prev_ = prev_ - 1;
		}

		prev_ = prev_ + 1;
		prev_->get()->add_pattern(std::move(map));
	};

};

}
