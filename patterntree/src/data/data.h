#pragma once

#include <memory>
#include <vector>

#include "data/data_concepts.h"
#include "data/value.h"
#include "data/backend/symbolic_value.h"
#include "data/backend/kokkos_value.h"

namespace PatternTree {

class IView;
class APT;

class IData {

protected:
	std::unique_ptr<IValue> value_;
	std::vector<size_t> split_size_;
	std::vector<std::shared_ptr<IView>> basis_;

public:
	friend class APT;

    IData(std::unique_ptr<IValue> value);

	/**
	 * Returns the underlying value.
	 *
	 * @return value
	 */
	IValue& value() const;

	/**
	 * Returns the identifier of the value.
	 *
	 * @return identifier
	 */
	std::string identifier() const;

	/**
	 * Returns the shape of the value.
	 *
	 * @return shape
	 */
	std::vector<size_t> shape() const;

	/**
	 * Returns a representation of the value over the basis views.
	 *
	 * @return basis
	 */
	std::vector<std::shared_ptr<IView>> basis() const;

	std::vector<size_t> split_size() const;

	/**
	 * Compiles the data, i.e., replace its symbolic values with actual memory.
	 */
	virtual void compile() = 0;

};

template<typename D>
class Data : public IData {

void compile_() requires ONEDIM<D>
{
	// TODO: Check if symbolic

	this->value_.reset(new KokkosValue<D>(this->identifier(), this->shape()[0]));
};

void compile_() requires TWODIM<D>
{
	// TODO: Check if symbolic

	this->value_.reset(new KokkosValue<D>(this->identifier(), this->shape()[0], this->shape()[1]));
};

public:
	Data(std::unique_ptr<IValue> value) : IData(std::move(value)) {};

	void compile() override { this->compile_(); };

	Value<D>& value() const
	{
		return *(static_cast<Value<D>*>(this->value_.get()));
	}

	remove_all_pointers_t<D>& operator () (size_t dim0) requires ONEDIM<D> {
        return this->value().operator()(dim0);
    };

    remove_all_pointers_t<D>& operator () (size_t dim0, size_t dim1) requires TWODIM<D> {
        return this->value().operator()(dim0, dim1);
    };
};

}