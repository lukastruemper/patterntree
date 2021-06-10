#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <math.h>

#include "data/data_concepts.h"

namespace PatternTree
{

class IView;

class IData {

std::string name_;
std::vector<int> shape_;

protected:
	bool symbolic_;

	std::vector<size_t> split_size_;
	std::vector<std::shared_ptr<IView>> basis_;

public:

	virtual ~IData() {};
	IData(std::string, int dim0, int dim1);

	std::string name() const;
	bool is_symbolic() const;
	std::vector<int> shape() const;

	std::vector<std::shared_ptr<IView>> basis() const
	{
		return this->basis_;
	};

	std::vector<size_t> split_size() const
	{
		return this->split_size_;
	}

};

template<typename D>
class Data : public IData {

public:
	friend class APT;

	Data(std::string name, int dim0) requires ONEDIM<D>
	: IData(name, dim0, 0) {}

	Data(std::string name, int dim0, int dim1) requires TWODIM<D>
	: IData(name, dim0, dim1)
	{}

};
}
