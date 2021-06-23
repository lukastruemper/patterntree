#include "apt.h"

#include <Kokkos_Core.hpp>

#include "optimization/optimizer.h"

using json = nlohmann::json;

PatternTree::APT* PatternTree::APT::instance = 0;

size_t PatternTree::APT::size() const
{
	return this->flow_.size();
}

const PatternTree::Cluster& PatternTree::APT::cluster() const
{
	return *(this->cluster_);
}

std::vector<std::shared_ptr<PatternTree::IData>> PatternTree::APT::data() const
{
	return this->data_;
}

void PatternTree::APT::optimize(PatternTree::IOptimizer& optimizer)
{
	auto begin_iter = this->begin();
	auto end_iter = this->end();

	optimizer.init(begin_iter, end_iter, this->cluster());
	for (auto iter = begin_iter; iter != end_iter; iter++)
	{
		optimizer.assign(iter);
		if (!iter->complete())
		{
			// ERROR
			std::cout << "Error" << std::endl;
			return;
		}
	}
};

double PatternTree::APT::evaluate(PatternTree::IPerformanceModel& model)
{
	for (auto iter = this->begin(); iter != this->end(); iter++)
	{
		model.update(*iter);
	}

	return model.costs();
}

nlohmann::json PatternTree::APT::to_json()
{
	json report = json::object();
	report["size"] = this->size();

	json steps = json::array();
	for (auto iter = this->begin(); iter != this->end(); iter++)
	{
		steps.push_back(iter->to_json());
	}
	report["steps"] = steps;

	return report;
}

void PatternTree::APT::init(std::shared_ptr<PatternTree::Cluster> cluster)
{
	PatternTree::APT::init(cluster, 2, 32, true);
};

void PatternTree::APT::init(std::shared_ptr<PatternTree::Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency)
{
	PatternTree::APT::init(cluster, operation_interpolation_frequency, data_interpolation_frequency, true);
};

void PatternTree::APT::init(std::shared_ptr<PatternTree::Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency, bool synchronization_efficiency)
{
	PatternTree::APT::instance = new PatternTree::APT(cluster, operation_interpolation_frequency, data_interpolation_frequency, synchronization_efficiency);
};

void PatternTree::APT::synchronization_efficiency(bool enabled)
{
	PatternTree::APT::instance->synchronization_efficiency_ = enabled;
}

void PatternTree::APT::synchronization_efficiency_length(int length)
{
	PatternTree::APT::instance->synchronization_efficiency_length_ = length;
}

void PatternTree::APT::operation_interpolation_frequency(size_t frequency)
{
	PatternTree::APT::instance->operation_interpolation_frequency_ = frequency;
}

void PatternTree::APT::data_interpolation_frequency(size_t frequency)
{
	PatternTree::APT::instance->data_interpolation_frequency_ = frequency;
}

std::unique_ptr<PatternTree::APT> PatternTree::APT::compile()
{
	Kokkos::initialize();

	for (auto data : instance->data_) {
		data->compile();
	}

	std::unique_ptr<PatternTree::APT> apt(PatternTree::APT::instance);
	PatternTree::APT::instance = 0;

	return apt;
};
