#include "apt.h"

#include <Kokkos_Core.hpp>

#include "optimization/optimizer.h"

PatternTree::APT* PatternTree::APT::instance = 0;

size_t PatternTree::APT::size()
{
	return this->flow_.size();
};

const PatternTree::Cluster& PatternTree::APT::cluster()
{
	return *(this->cluster_);
};

const std::vector<std::shared_ptr<PatternTree::IData>>& PatternTree::APT::sources()
{
	return this->sources_;
};

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

	std::cout << "Final costs: " << optimizer.costs() << std::endl;
};

double PatternTree::APT::evaluate(PatternTree::IPerformanceModel& model)
{
	for (auto iter = this->begin(); iter != this->end(); iter++)
	{
		model.update(*iter);
	}

	std::cout << "Final runtime: " << model.runtime() << std::endl;

	return model.runtime();
}

void PatternTree::APT::summary()
{
	std::cout << "APT( ";
	std::cout << "sources: " << this->sources_.size() << " ,";
	std::cout << "steps: " << this->flow_.size() << " )" << std::endl;
	std::cout << std::endl;
};

void PatternTree::APT::initialize(std::shared_ptr<PatternTree::Cluster> cluster)
{
	PatternTree::APT::initialize(cluster, 2, 32, true);
};

void PatternTree::APT::initialize(std::shared_ptr<PatternTree::Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency)
{
	PatternTree::APT::initialize(cluster, operation_interpolation_frequency, data_interpolation_frequency, true);
};

void PatternTree::APT::initialize(std::shared_ptr<PatternTree::Cluster> cluster, size_t operation_interpolation_frequency, size_t data_interpolation_frequency, bool synchronization_efficiency)
{
	PatternTree::APT::instance = new PatternTree::APT(cluster, operation_interpolation_frequency, data_interpolation_frequency, synchronization_efficiency);
	
	Kokkos::initialize();
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
	std::unique_ptr<PatternTree::APT> apt(PatternTree::APT::instance);
	PatternTree::APT::instance = 0;

	return apt;
};
