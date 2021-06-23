#pragma once

#include <string>
#include <vector>
#include <memory>

#include "data/data_concepts.h"

namespace PatternTree
{

class IValue {

std::string identifier_;
std::vector<size_t> shape_;

public:

	virtual ~IValue() {};
	IValue(std::string identifier, size_t dim0, size_t dim1);

	/**
	 * Returns the identifier of the data.
	 *
	 * @return identifier
	 */
	std::string identifier() const;

	/**
	 * Returns the shape of the data.
	 *
	 * @return shape
	 */
	std::vector<size_t> shape() const;

};

template<typename D>
class Value : public IValue {

public:
	Value(std::string identifier, size_t dim0, size_t dim1)
	: IValue(identifier, dim0, dim1) {};

	/***** VALUE: Member functions *****/

	virtual remove_all_pointers_t<D>& at(size_t dim0, size_t dim1) = 0;

	/***** VALUE: OPERATOR *****/

	remove_all_pointers_t<D>& operator () (size_t dim0) requires ONEDIM<D>
	{
		return this->at(dim0, 0);
	};

    remove_all_pointers_t<D>& operator () (size_t dim0, size_t dim1) requires TWODIM<D>
	{
		return this->at(dim0, dim1);
	};

};

}
