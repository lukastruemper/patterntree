#pragma once

#include <Kokkos_Core.hpp>

#include "data/data_concepts.h"
#include "data/value.h"

namespace PatternTree {

template<typename D>
class KokkosValue : public Value<D> {

Kokkos::View<D> value_;

public:
	KokkosValue(std::string identifier, size_t dim0) requires ONEDIM<D>
	: Value<D>(identifier, dim0, 0)
	{
		this->value_ = Kokkos::View<D>(identifier, dim0);
	};

	KokkosValue(std::string identifier, size_t dim0, size_t dim1) requires TWODIM<D>
	: Value<D>(identifier, dim0, dim1)
	{
		this->value_ = Kokkos::View<D>(identifier, dim0, dim1);
	};

	remove_all_pointers_t<D>& at(size_t dim0, size_t dim1) override
	{
		if constexpr (ONEDIM<D>) {
			return this->value_(dim0);
		} else {
			return this->value_(dim0, dim1);
		}
	};


};

}