#pragma once

#include "data/data_concepts.h"
#include "data/value.h"

namespace PatternTree {

template<typename D>
class SymbolicValue : public Value<D> {

remove_all_pointers_t<D> dummy_;

public:
	SymbolicValue(std::string identifier, size_t dim0) requires ONEDIM<D>
	: Value<D>(identifier, dim0, 0), dummy_(0) {}

	SymbolicValue(std::string identifier, size_t dim0, size_t dim1) requires TWODIM<D>
	: Value<D>(identifier, dim0, dim1), dummy_(0) {}

	remove_all_pointers_t<D>& at(size_t dim0, size_t dim1) override
	{
		return this->dummy_;
	};

};

}