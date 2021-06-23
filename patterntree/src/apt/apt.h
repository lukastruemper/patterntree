#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <math.h> 

#include <ctime>
#include <unistd.h>

#include <iterator>
#include <cstddef> 

#include <nlohmann/json.hpp>

#include "apt/step.h"
#include "data/data.h"
#include "data/value.h"
#include "data/backend/symbolic_value.h"
#include "data/backend/kokkos_value.h"

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

std::vector<std::shared_ptr<IData>> data_;
std::vector<std::unique_ptr<Step>> flow_;

public:
	
	/***** APT: Methods *****/

	size_t size() const;

	const Cluster& cluster() const;

	std::vector<std::shared_ptr<IData>> data() const;
			
	void optimize(IOptimizer& optimizer);
	
	double evaluate(IPerformanceModel& model);

	nlohmann::json to_json();
	
	/***** APT: ITERATOR *****/
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

	APT::Iterator begin() { return Iterator( this->flow_.begin() ); }
    APT::Iterator end()   { return Iterator( this->flow_.end() ); }

	/***** APT: INIT & COMPILE *****/

	static void init(std::shared_ptr<Cluster> cluster);
	static void init(std::shared_ptr<Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency);
	static void init(std::shared_ptr<Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency, bool synchronization_efficiency);

	static std::unique_ptr<APT> compile();

	/***** APT: HYPERPARAMETERS *****/

	static void operation_interpolation_frequency(size_t frequency);
	static void data_interpolation_frequency(size_t frequency);
	static void synchronization_efficiency(bool enabled);
	static void synchronization_efficiency_length(int length);

	/***** APT: DATA *****/

	template<typename D>
	static std::shared_ptr<View<D>> data(std::string identifier, size_t dim0) requires ONEDIM<D>
	{
		return APT::data<D>(identifier, dim0, APT::instance->data_interpolation_frequency_);
	}

	template<typename D>
	static std::shared_ptr<View<D>> data(std::string identifier, size_t dim0, size_t interpolation_frequency) requires ONEDIM<D>
	{
		std::shared_ptr<Data<D>> data = APT::symbolic<D>(identifier, dim0, interpolation_frequency);
		APT::instance->data_.push_back(data);

		auto view = View<D>::full(data);
		return view;
	}

	template<typename D>
	static std::shared_ptr<View<D>> data(std::string identifier, size_t dim0, size_t dim1) requires TWODIM<D>
	{
		return APT::data<D>(identifier, dim0, dim1, APT::instance->data_interpolation_frequency_);
	}

	template<typename D>
	static std::shared_ptr<View<D>> data(std::string identifier, size_t dim0, size_t dim1, size_t interpolation_frequency) requires TWODIM<D>
	{
		std::shared_ptr<Data<D>> data = APT::symbolic<D>(identifier, dim0, dim1, interpolation_frequency);
		APT::instance->data_.push_back(data);

		auto view = View<D>::full(data);
		return view;
	}

	template<typename D>
	static std::shared_ptr<Data<D>> symbolic(std::string identifier, size_t dim0, size_t interpolation_frequency) requires ONEDIM<D>
	{
		std::unique_ptr<SymbolicValue<D>> value(new SymbolicValue<D>(identifier, dim0));
		std::shared_ptr<Data<D>> data(new Data<D>(std::move(value)));

		size_t split_size_0 = std::max(dim0 / interpolation_frequency, interpolation_frequency);
		data->split_size_.push_back(split_size_0);
		for (size_t i = 0; i < dim0; i += split_size_0)
		{
			size_t length_0 = std::min(split_size_0, dim0 - i);

			std::shared_ptr<View<D>> basis_view = View<D>::slice(data, std::make_pair(i, i + length_0));
			data->basis_.push_back(basis_view);
		}

		return data;
	};

	template<typename D>
	static std::shared_ptr<Data<D>> symbolic(std::string identifier, size_t dim0, size_t dim1, size_t interpolation_frequency) requires TWODIM<D>
	{
		std::unique_ptr<SymbolicValue<D>> value(new SymbolicValue<D>(identifier, dim0, dim1));
		std::shared_ptr<Data<D>> data(new Data<D>(std::move(value)));

		size_t split_size_0_ = std::max(dim0 / interpolation_frequency, interpolation_frequency);
		size_t split_size_1_ = std::max(dim1 / interpolation_frequency, interpolation_frequency);
		data->split_size_.push_back(split_size_0_);
		data->split_size_.push_back(split_size_1_);
		for (size_t i = 0; i < dim0; i += split_size_0_)
		{
			size_t length_0 = std::min(split_size_0_, dim0 - i);
			for (size_t j = 0; j < dim1; j += split_size_1_)
			{
				size_t length_1 = std::min(split_size_1_, dim1 - j);

				std::shared_ptr<View<D>> basis_view = View<D>::slice(data, std::make_pair(i, i + length_0), std::make_pair(j, j + length_1));
				data->basis_.push_back(basis_view);
			}
		}

		return data;
	};

	/***** APT: PATTERNS *****/

	template<typename D, typename Functor>
	static void map(std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field) requires MAPFUNCTOR<Functor, D>
	{
		size_t frequency = instance->operation_interpolation_frequency_;
		map(std::move(functor), field, frequency);
	}

	template<typename D, typename Functor>
	static void map(std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field, size_t interpolation_frequency) requires MAPFUNCTOR<Functor, D>
	{
    	static const char alphanum[] =
        	"0123456789"
        	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        	"abcdefghijklmnopqrstuvwxyz";
    
    	srand( (unsigned) time(NULL) * getpid());

		std::string identifier;
    	identifier.reserve(18);

    	for (int i = 0; i < 18; ++i) 
        	identifier += alphanum[rand() % (sizeof(alphanum) - 1)];

		map(identifier, std::move(functor), field, interpolation_frequency);
	}

	template<typename D, typename Functor>
	static void map(std::string identifier, std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field) requires MAPFUNCTOR<Functor, D>
	{
		size_t frequency = instance->operation_interpolation_frequency_;
		map(identifier, std::move(functor), field, frequency);
	}

	template<typename D, typename Functor>
	static void map(std::string identifier, std::unique_ptr<Functor> functor, std::shared_ptr<View<D>> field, size_t interpolation_frequency) requires MAPFUNCTOR<Functor, D>
	{
		auto map = Map<D>::create(identifier, std::move(functor), field, interpolation_frequency);
		
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
