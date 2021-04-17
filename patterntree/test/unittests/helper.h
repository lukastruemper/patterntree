#pragma once

#include <data/view.h>
#include <patterns/map.h>
#include <api/arithmetic.h>

struct DummyMapFunctor : public PatternTree::MapFunctor<double*> {
    void operator () (const int index, PatternTree::View<double*>& element) override {
        return;
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};
};

struct TwoViewsMapFunctor : public PatternTree::MapFunctor<double*> {
    TwoViewsMapFunctor(std::shared_ptr<PatternTree::View<double*>> second_view) : second_view_(second_view)
    {}
    
    void operator () (const int index, PatternTree::View<double*>& element) override {
        return;
    };

    void consumes(PatternTree::Dataflow& dataflow) override {
        dataflow.push_back(second_view_);
    };

private:
    std::shared_ptr<PatternTree::View<double*>> second_view_;
};

template<typename T>
struct TemplateMapFunctor : public PatternTree::MapFunctor<T> {
    void operator () (const int index, PatternTree::View<T>& element) override {
        return;
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};
};

struct ConstantCostsMapFunctor : public PatternTree::MapFunctor<double*> {
    void operator () (const int i, PatternTree::View<double*>& v) override
    {
        v = v + 1;
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};
};

struct TriangleCostsMapFunctor : public PatternTree::MapFunctor<double**> {
    void operator () (const int i, PatternTree::View<double**>& v) override
    {
        for (int j = 0; j <= i; j++)
        {
            v = v + 1;
        }
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};
};

struct CustomTriangleCostsMapFunctor : public PatternTree::MapFunctor<double**> {
    void operator () (const int i, PatternTree::View<double**>& v) override
    {
        for (int j = 0; j <= i; j++)
        {
            v = v + 1;
        }
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    {
        info.flops = index * 2 + 2;

        info.flops += 1;

        return true;
    }
};

struct SplitMapFunctor : public PatternTree::MapFunctor<double*> {
    
    SplitMapFunctor(std::shared_ptr<PatternTree::View<double*>> second_view) : second_view_(second_view)
    {}
    
    void operator () (const int i, PatternTree::View<double*>& v) override
    {
        for (int j = 0; j <= i; j++)
        {
            v = v + 1;
        }
    };

    void consumes(PatternTree::Dataflow& dataflow) override {
        dataflow.push_back(second_view_);
    };
    
    bool touch(const int index, PatternTree::PatternIndexInfo &info) override {
        info.subviews.insert({
            second_view_->data().lock().get(),
            PatternTree::View<double*>::element(second_view_->data(), index)
        });
        info.flops = index + 1;

        return true;
    }
    
private:
    std::shared_ptr<PatternTree::View<double*>> second_view_;
};
