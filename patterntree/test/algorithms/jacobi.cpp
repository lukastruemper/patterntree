#pragma once

#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>


struct JacobiFunctor : public PatternTree::MapFunctor<double*> {
    JacobiFunctor(size_t N, std::shared_ptr<PatternTree::View<double**>> A, std::shared_ptr<PatternTree::View<double*>> b, std::shared_ptr<PatternTree::View<double*>> x_)
    : N(N), A(A), b(b), x_(x_)
    {}

    void operator () (const int index, PatternTree::View<double*>& x) override {
        x = (*b)(index);
        for (size_t j = 0; j < N; j++)
        {
            if (j != index)
                x = x - (*A)(index, j) * (*x_)(j);
        }
        x = x / (*A)(index, index);
    };

    void consumes(PatternTree::Dataflow& dataflow) override {
        dataflow.push_back(A);
        dataflow.push_back(b);
        dataflow.push_back(x_);
    };

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    {
        // Loop: Check & increment
        info.flops = 2 * N;
        // Check i != j and update x
        info.flops += N * 3;
        // Divison
        info.flops += 1;
        return true;
    }

private:
    size_t N;
    std::shared_ptr<PatternTree::View<double**>> A;
    std::shared_ptr<PatternTree::View<double*>> b;
    std::shared_ptr<PatternTree::View<double*>> x_;
};

std::unique_ptr<PatternTree::APT> jacobi(size_t N, size_t K, bool fuse)
{
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::init(cluster, 2, 2, true);

    auto A = PatternTree::APT::data<double**>("A", N, N);
    auto A_upper = PatternTree::View<double**>::slice(A->data(), std::make_pair(0, N / 2), std::make_pair(0, N));
    auto A_lower = PatternTree::View<double**>::slice(A->data(), std::make_pair(N / 2, N), std::make_pair(0, N));

    // Jacobi 1

    auto b = PatternTree::APT::data<double*>("b", N);
    auto b_upper = PatternTree::View<double*>::slice(b->data(),  std::make_pair(0, N / 2));
    auto b_lower = PatternTree::View<double*>::slice(b->data(), std::make_pair(N / 2, N));

    auto x = PatternTree::APT::data<double*>("x", N);
    auto x_ = PatternTree::APT::data<double*>("x_", N);

    for (int k = 0; k < K; k++)
    {
        auto x_lower = PatternTree::View<double*>::slice(x->data(),  std::make_pair(0, N / 2));
        auto x_upper = PatternTree::View<double*>::slice(x->data(),  std::make_pair(N / 2, N));

        std::unique_ptr<JacobiFunctor> functor_upper(new JacobiFunctor(N / 2, A_upper, b_upper, x_));    
        PatternTree::APT::map<double*, JacobiFunctor>("jacobi_upper_1", std::move(functor_upper), x_upper);

        std::unique_ptr<JacobiFunctor> functor_lower(new JacobiFunctor(N / 2, A_lower, b_lower, x_));    
        PatternTree::APT::map<double*, JacobiFunctor>("jacobi_lower_1", std::move(functor_lower), x_lower);

        x.swap(x_);
    }

    // Jacobi 2

    auto b2 = PatternTree::APT::data<double*>("b2", N);
    auto b_upper2 = PatternTree::View<double*>::slice(b2->data(),  std::make_pair(0, N / 2));
    auto b_lower2 = PatternTree::View<double*>::slice(b2->data(), std::make_pair(N / 2, N));

    auto x2 = PatternTree::APT::data<double*>("x2", N);
    auto x_2 = PatternTree::APT::data<double*>("x_2", N);

    for (int k = 0; k < K; k++)
    {
        PatternTree::APT::synchronization_efficiency(fuse);
        PatternTree::APT::synchronization_efficiency_length(-1);

        auto x_lower2 = PatternTree::View<double*>::slice(x2->data(), std::make_pair(0, N / 2));
        auto x_upper2 = PatternTree::View<double*>::slice(x2->data(), std::make_pair(N / 2, N));

        std::unique_ptr<JacobiFunctor> functor_upper(new JacobiFunctor(N / 2, A_upper, b_upper2, x_2));    
        PatternTree::APT::map<double*, JacobiFunctor>("jacobi_upper_2", std::move(functor_upper), x_upper2);

        if (!fuse)
        {
            PatternTree::APT::synchronization_efficiency(true);
            PatternTree::APT::synchronization_efficiency_length(1);
        }

        std::unique_ptr<JacobiFunctor> functor_lower(new JacobiFunctor(N / 2, A_lower, b_lower2, x_2));    
        PatternTree::APT::map<double*, JacobiFunctor>("jacobi_lower_2", std::move(functor_lower), x_lower2);

        x2.swap(x_2);

    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    return std::move(apt);
};

class JacobiMappingBase : public PatternTree::IOptimizer {
public:
    JacobiMappingBase() {};

    void init(PatternTree::APT::Iterator begin, PatternTree::APT::Iterator end, const PatternTree::Cluster& cluster) override {
        std::shared_ptr<PatternTree::Node> node = cluster.nodes().begin()->second;
        std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
        std::shared_ptr<PatternTree::Processor> processorA = (device->processors().begin())->second;
        std::shared_ptr<PatternTree::Processor> processorB = (++(device->processors().begin()))->second;

        std::unique_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 24));
        std::unique_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 24));
        teamA_ = std::move(teamA);
        teamB_ = std::move(teamB);
    };

    void assign(PatternTree::APT::Iterator& step) override {
        auto it = step->begin();
        int size = step->size();
        for (int i = 0; i < size; i++)
        {
            PatternTree::IPattern& split = *it;
            if (i < size / 2) {
                step->assign(split, teamA_);
            } else {
                step->assign(split, teamB_);
            }

            it++;
        }
    };

    double costs() override {
        return -1.0;
    };

    std::shared_ptr<PatternTree::Team> teamA_;
    std::shared_ptr<PatternTree::Team> teamB_;
};

TEST(TestSuiteJacobi, TestBase)
{
    size_t N = 8192;
    size_t K = 50;
    auto apt = jacobi(N, K, false);

    JacobiMappingBase mapping;
    apt->optimize(mapping);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    double runtime_ratio = runtime / 0.986;
    ASSERT_TRUE(runtime_ratio < 3 && runtime_ratio > 0.3333);
}

class JacobiMappingOptimized : public PatternTree::IOptimizer {
public:
    JacobiMappingOptimized() {};

    void init(PatternTree::APT::Iterator begin, PatternTree::APT::Iterator end, const PatternTree::Cluster& cluster) override {
        std::shared_ptr<PatternTree::Node> node = cluster.nodes().begin()->second;
        std::shared_ptr<PatternTree::Device> device = node->devices().find("CPU1")->second;
        std::shared_ptr<PatternTree::Processor> processorA = (device->processors().begin())->second;
        std::shared_ptr<PatternTree::Processor> processorB = (++(device->processors().begin()))->second;

        std::unique_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 24));
        std::unique_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 24));
        teamA_ = std::move(teamA);
        teamB_ = std::move(teamB);
    };

    void assign(PatternTree::APT::Iterator& step) override {
        auto it = step->begin();
        for (int i = 0; i < 4; i++)
        {
            PatternTree::IPattern& split = *it;
            if (i == 0 || i == 2) {
                step->assign(split, teamA_);
            } else {
                step->assign(split, teamB_);
            }

            it++;
        }
    };

    double costs() override {
        return -1.0;
    };

    std::shared_ptr<PatternTree::Team> teamA_;
    std::shared_ptr<PatternTree::Team> teamB_;
};

TEST(TestSuiteJacobi, TestOptimized)
{
    size_t N = 8192;
    size_t K = 50;
    auto apt = jacobi(N, K, true);

    JacobiMappingOptimized mapping;
    apt->optimize(mapping);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    double runtime_ratio = runtime / 0.539;
    ASSERT_TRUE(runtime_ratio < 3 && runtime_ratio > 0.3333);
}
