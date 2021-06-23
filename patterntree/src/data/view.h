#pragma once

#include <math.h>
#include <memory>
#include <vector>
#include <unordered_set>

#include "data/data_concepts.h"
#include "data/data.h"

namespace PatternTree
{
class IView {

bool nested_;

protected:
    std::weak_ptr<IData> data_;

    std::vector<size_t> shape_;
    std::vector<size_t> begins_;
    std::vector<size_t> ends_;

public:
    IView(std::weak_ptr<IData> data, std::pair<size_t, size_t> dim0, std::pair<size_t, size_t> dim1);

	/**
	 * Returns a reference to the underlying data.
	 *
	 * @return data
	 */
    std::weak_ptr<IData> data();

	/**
	 * Returns the shape of the view.
	 *
	 * @return shape
	 */
    std::vector<size_t> shape() const;

	/**
	 * Returns the absolute leading offset for each dimension.
	 *
	 * @return begins
	 */
    std::vector<size_t> begins() const;

	/**
	 * Returns the absolute trailing offset for each dimension.
	 *
	 * @return ends
	 */
    std::vector<size_t> ends() const;

	/**
	 * Returns the total number of elements covered by this view.
	 *
	 * @return elements
	 */
    size_t elements() const;

	/**
	 * Returns the kbytes occupied by this view.
	 *
	 * @return kbytes
	 */
    double kbytes() const;

	/**
	 * Clones the view.
	 *
	 * @return view.
	 */
    virtual std::shared_ptr<IView> clone() const = 0;
    
    /**
	 * Determines whether this and a given view are disjoint.
	 *
	 * @return disjoint
	 */
    virtual bool disjoint(IView& view) = 0;

    static std::shared_ptr<IView> join(IView& view1, IView& view2);

    static std::unordered_set<std::shared_ptr<IView>> as_basis(IView& view);


    void set_nested_context(bool nested)
    {
        this->nested_ = nested;
    };

    void add_FLOPS(int flops)
    {

    };

    int reset_FLOPS()
    {
        return 0;
    };

};

template<typename D>
class View: public IView {

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
    View(std::weak_ptr<Data<D>> data, std::pair<size_t, size_t> dim0) requires ONEDIM<D>
        : IView(data, dim0, std::make_pair(0, 0)) {};

    View(std::weak_ptr<Data<D>> data, std::pair<size_t, size_t> dim0, std::pair<size_t, size_t> dim1) requires TWODIM<D>
        : IView(data, dim0, dim1) {};

    std::weak_ptr<Data<D>> data() const
    {
        return std::static_pointer_cast<Data<D>>(this->data_.lock());
    };

    std::shared_ptr<IView> clone() const override
    {
        return this->clone_();
    };

    remove_all_pointers_t<D>& operator () (size_t dim0) requires ONEDIM<D> {
        return this->data().lock()->operator()(this->begins_[0] + dim0);
    };

    remove_all_pointers_t<D>& operator () (size_t dim0, size_t dim1) requires TWODIM<D> {
        return this->data().lock()->operator()(this->begins_[0] + dim0, this->begins_[1] + dim1);
    };

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
        return lhs;
    }

    friend View<D>& operator +(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        return lhs;
    }

    friend View<D>& operator +(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs + lhs;
    }

    friend View<D>& operator -(View<D>& lhs, const View<D>& rhs)
    {
        return lhs;
    }

    friend View<D>& operator -(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        return lhs;
    }

    friend View<D>& operator -(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs - lhs;
    }

        friend View<D>& operator *(View<D>& lhs, const View<D>& rhs)
    {
        return lhs;
    }

    friend View<D>& operator *(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        return lhs;
    }

    friend View<D>& operator *(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs * lhs;
    }

        friend View<D>& operator /(View<D>& lhs, const View<D>& rhs)
    {
        return lhs;
    }

    friend View<D>& operator /(View<D>& lhs, remove_all_pointers_t<D> rhs)
    {
        return lhs;
    }

    friend View<D>& operator /(remove_all_pointers_t<D> lhs, View<D>& rhs)
    {
        return rhs / lhs;
    }

    static std::shared_ptr<View<D>> slice(std::weak_ptr<Data<D>> data, std::pair<size_t,size_t> dim0) requires ONEDIM<D>
    {
        return std::shared_ptr<View<D>>(new View<D>(data, dim0));
    };

    static std::shared_ptr<View<D>> slice(std::weak_ptr<Data<D>> data, std::pair<size_t,size_t> dim0, std::pair<size_t,size_t> dim1) requires TWODIM<D>
    {
        return std::shared_ptr<View<D>>(new View<D>(data, dim0, dim1));
    };

    static std::shared_ptr<View<D>> element(std::weak_ptr<Data<D>> data, size_t index) requires ONEDIM<D>
    {
        return View<D>::slice(data, std::make_pair(index, index + 1));
    };

    static std::shared_ptr<View<D>> element(std::weak_ptr<Data<D>> data, size_t index) requires TWODIM<D>
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

        size_t begin_0 = std::min(view1.begins_[0], view2.begins_[0]);
        size_t end_0 = std::max(view1.ends_[0], view2.ends_[0]);
        return View<D>::slice(view1.data(), std::make_pair(begin_0, end_0));
    }

    static std::shared_ptr<View<D>> join(View<D>& view1, View<D>& view2) requires TWODIM<D>
    {
        if (view1.data().lock() != view2.data().lock()) {
            // error
            return nullptr;
        }

        size_t begin_0 = std::min(view1.begins_[0], view2.begins_[0]);
        size_t begin_1 = std::min(view1.begins_[1], view2.begins_[1]);
        size_t end_0 = std::max(view1.ends_[0], view2.ends_[0]);
        size_t end_1 = std::max(view1.ends_[1], view2.ends_[1]);
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
