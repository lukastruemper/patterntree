#pragma once

#include <vector>
#include <memory>
#include <type_traits>

#include <Kokkos_Core.hpp>

#include "data/data.h"
#include "data/data_concepts.h"

namespace PatternTree
{
class IView {

int flops_;
bool nested_context_;
std::vector<int> shape_;

protected:
    std::weak_ptr<IData> data_;

    std::vector<int> begins_;
    std::vector<int> ends_;

public:
    IView(std::weak_ptr<IData> data, std::pair<int, int> dim0, std::pair<int, int> dim1)
    : data_(data), flops_(0), nested_context_(false)
    {
        begins_.push_back(dim0.first);
        ends_.push_back(dim0.second);
        shape_.push_back(dim0.second - dim0.first);
        if (dim1.first >= 0)
        {
            begins_.push_back(dim1.first);
            ends_.push_back(dim1.second);
            shape_.push_back(dim1.second - dim1.first);
        }
    };

    std::weak_ptr<IData> data();
    std::string name() const;
    std::vector<int> shape() const;
    std::vector<int> begins() const;
    std::vector<int> ends() const;
    int elements() const;
    bool is_symbolic() const;
    bool is_nested_context() const;
    double kbytes() const;

    void set_nested_context(bool);
    void add_FLOPS(int);
    int reset_FLOPS();

    virtual std::shared_ptr<IView> clone() const = 0;
    virtual bool disjoint(IView& view) = 0;

    static std::shared_ptr<IView> join(IView& view1, IView& view2)
    {
        if (view1.data().lock() != view2.data().lock()) {
            // error
            return nullptr;
        }

        int begin_0 = std::min(view1.begins_[0], view2.begins_[0]);
        int end_0 = std::max(view1.ends_[0], view2.ends_[0]);

        std::shared_ptr<IView> joined = view1.clone();
        joined->begins_ = { begin_0 };
        joined->ends_ = { end_0 };
        joined->shape_ = { end_0 - begin_0 };

        if (view1.shape_.size() == 2) {
            int end_1 = std::max(view1.ends_[1], view2.ends_[1]);
            int begin_1 = std::min(view1.begins_[1], view2.begins_[1]);

            joined->begins_.push_back(begin_1);
            joined->ends_.push_back(end_1);
            joined->shape_.push_back(end_1 - begin_1);
        }

        return joined;
    }

    static std::unordered_set<std::shared_ptr<IView>> as_basis(IView& view)
	{
		auto data = view.data().lock();
		std::unordered_set<std::shared_ptr<IView>> view_basis;

		std::vector<int> begins = view.begins();
		std::vector<int> ends = view.ends();

		size_t split_0 = data->split_size()[0];
		int start_0 = floor(begins[0] / (float) split_0);
		int end_0 = ceil(ends[0] / (float) split_0);
		for (int i = start_0; i < end_0; i++)
		{
			if (begins.size() == 1)
			{
				view_basis.insert(data->basis().at(i));
			} else {
				size_t split_1 = data->split_size()[1];
				int start_1 = floor(begins[1] / (float) split_1);
				int end_1 = ceil(ends[1] / (float) split_1);

				size_t length_1 = ceil(data->shape()[1] / (float) split_1);
				for (int j = start_1; j < end_1; j++)
				{
					view_basis.insert(data->basis().at(i * length_1 + j));
				}
			}
		}

		return view_basis;
	};

};

template<typename D>
class View: public IView {

remove_all_pointers_t<D> dummy_;

std::shared_ptr<View<D>> clone_() const requires ONEDIM<D> 
{
    std::shared_ptr<View<D>> view(new View<D>(
        this->data(),
        std::make_pair(this->begins_.at(0), this->ends_.at(0))
    ));

    return view;
};

std::shared_ptr<View<D>> clone_() const requires TWODIM<D>
{
    std::shared_ptr<View<D>> view(new View<D>(
        this->data(),
        std::make_pair(this->begins_.at(0), this->ends_.at(0)),
        std::make_pair(this->begins_.at(1), this->ends_.at(1))
    ));

    return view;
};

public:
    View(std::weak_ptr<Data<D>> data, std::pair<int, int> dim0) requires ONEDIM<D>
        : IView(data, dim0, std::make_pair(-1, -1)),
        dummy_(0)
    {};

    View(std::weak_ptr<Data<D>> data, std::pair<int, int> dim0, std::pair<int, int> dim1) requires TWODIM<D>
        : IView(data, dim0, dim1),
        dummy_(0)
    {};

    std::weak_ptr<Data<D>> data() const
    {
        return std::static_pointer_cast<Data<D>>(this->data_.lock());
    }

    std::shared_ptr<IView> clone() const override
    {
        return this->clone_();
    };

    remove_all_pointers_t<D>& operator () (int dim0) requires ONEDIM<D> {
        this->dummy_ = 0;
        return dummy_;
    }

    remove_all_pointers_t<D>& operator () (int dim0, int dim1) requires TWODIM<D> {
        this->dummy_ = 0;
        return dummy_;
    }

    View<D>& operator =(const View<D>& rhs)
    {
        return *this;
    }

    View<D>& operator =(remove_all_pointers_t<D> rhs)
    {
        return *this;
    }

    friend View<D>& operator +(View<D>& lhs, const View<D>& rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator +(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator +(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs + lhs;
    }

    friend View<D>& operator -(View<D>& lhs, const View<D>& rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator -(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator -(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs - lhs;
    }

        friend View<D>& operator *(View<D>& lhs, const View<D>& rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator *(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator *(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs * lhs;
    }

        friend View<D>& operator /(View<D>& lhs, const View<D>& rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator /(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        lhs.add_FLOPS(1 * lhs.elements());
        return lhs;
    }

    friend View<D>& operator /(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs / lhs;
    }

    static std::shared_ptr<View<D>> slice(std::weak_ptr<Data<D>> data, std::pair<int,int> dim0) requires ONEDIM<D>
    {
        return std::shared_ptr<View<D>>(new View<D>(data, dim0));
    };

    static std::shared_ptr<View<D>> slice(std::weak_ptr<Data<D>> data, std::pair<int,int> dim0, std::pair<int,int> dim1) requires TWODIM<D>
    {
        return std::shared_ptr<View<D>>(new View<D>(data, dim0, dim1));
    };

    static std::shared_ptr<View<D>> element(std::weak_ptr<Data<D>> data, int index) requires ONEDIM<D>
    {
        return View<D>::slice(data, std::make_pair(index, index + 1));
    };

    static std::shared_ptr<View<D>> element(std::weak_ptr<Data<D>> data, int index) requires TWODIM<D>
    {
        return View<D>::slice(data, std::make_pair(index, index + 1), std::make_pair(0, data.lock()->shape().at(1)));
    };

    static std::shared_ptr<View<D>> full(std::weak_ptr<Data<D>> data) requires ONEDIM<D>
    {
        return View<D>::slice(data, std::make_pair(0, data.lock()->shape().at(0)));
    };

    static std::shared_ptr<View<D>> full(std::weak_ptr<Data<D>> data) requires TWODIM<D>
    {
        return View<D>::slice(data, std::make_pair(0, data.lock()->shape().at(0)), std::make_pair(0, data.lock()->shape().at(1)));
    };

    static std::shared_ptr<View<D>> join(View<D>& view1, View<D>& view2) requires ONEDIM<D>
    {
        if (view1.data().lock() != view2.data().lock()) {
            // error
            return nullptr;
        }

        int begin_0 = std::min(view1.begins_[0], view2.begins_[0]);
        int end_0 = std::max(view1.ends_[0], view2.ends_[0]);
        return View<D>::slice(view1.data(), std::make_pair(begin_0, end_0));
    }

    static std::shared_ptr<View<D>> join(View<D>& view1, View<D>& view2) requires TWODIM<D>
    {
        if (view1.data().lock() != view2.data().lock()) {
            // error
            return nullptr;
        }

        int begin_0 = std::min(view1.begins_[0], view2.begins_[0]);
        int begin_1 = std::min(view1.begins_[1], view2.begins_[1]);
        int end_0 = std::max(view1.ends_[0], view2.ends_[0]);
        int end_1 = std::max(view1.ends_[1], view2.ends_[1]);
        return View<D>::slice(view1.data(), std::make_pair(begin_0, end_0), std::make_pair(begin_1, end_1));
    }

    bool disjoint(IView& view) override
    {
        if (View<D>* v = dynamic_cast<View<D>*>(&view))
        {
            return this->disjoint(*v);
        }

        return true;
    };

    template<typename T>
    constexpr bool disjoint(View<T>& view)
    {
        return !std::is_same<D, T>::value;
    };

    bool disjoint(View<D>& view) requires ONEDIM<D>
    {
        if (this->data().lock() != view.data().lock()) { return true; }

        // O(1) heuristic: If not overlapping -> return false
        // TODO: extend bounds with split sizes 
        //bool overlapping = (this->ends_[0] > view.begins_[0] && this->begins_[0] < view.ends_[0])
        //|| (view.ends_[0] > this->begins_[0] && view.begins_[0] < this->ends_[0]);

        auto basis_lhs = IView::as_basis(*this);
        auto basis_rhs = IView::as_basis(view);

        for (auto const& basis_view : basis_rhs)
        {
            if (basis_lhs.find(basis_view) != basis_lhs.end())
            {
                return false;
            }
        }

        return true;
    };

    bool disjoint(View<D>& view) requires TWODIM<D>
    {
        if (this->data().lock() != view.data().lock()) { return true; }

        // O(1) heuristic: If not overlapping -> return false
        // TODO: extend bounds with split sizes 
        //bool overlapping = !(((this->ends_[0] > view.begins_[0] && this->begins_[0] < view.ends_[0])
        //    || (view.ends_[0] > this->begins_[0] && view.begins_[0] < this->ends_[0]))
        //&& 
        //    (((this->ends_[1] > view.begins_[1] && this->begins_[1] < view.ends_[1])
        //    || (view.ends_[1] > this->begins_[1] && view.begins_[1] < this->ends_[1]))));

        auto basis_lhs = IView::as_basis(*this);
        auto basis_rhs = IView::as_basis(view);

        for (auto const& basis_view : basis_rhs)
        {
            if (basis_lhs.find(basis_view) != basis_lhs.end())
            {
                return false;
            }
        }

        return true;
    };

};
}
