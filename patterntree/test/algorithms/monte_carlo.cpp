#pragma once

#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

struct MonteCarloFunctor : public PatternTree::MapFunctor<double*> {
    MonteCarloFunctor(long N) : N(N) {}

    long lehmer_random_number_generator(long x) {
        return (16807 * x) % 2147483647;
    }

    double uniform(long x) {
        return x / 2147483647.0;
    }

    void operator () (const int index, PatternTree::View<double*>& x) override {
        long sx = index * 2133;
        long sy = index * 33;

        long inside = 0;
        long outside = 0;
        for (long i = 0; i < N; i++) {
            sx = lehmer_random_number_generator(sx);
            sy = lehmer_random_number_generator(sy);
            double x = uniform(sx);
            double y = uniform(sy);

            double k = x * x + y * y;
            if (k <= 1.0) {
                inside += 1;
            } else {
                outside += 1;
            }
        }

        x(index) = (4.0 * inside) / (inside + outside);
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    {
        info.flops = 2;
        
        info.flops += 2 * N;
        
        info.flops += 2 * N;
        info.flops += 2 * N;
        info.flops += N;
        info.flops += N;

        info.flops += 3 * N;
        info.flops += 2 * N;
        
        info.flops += 3;
        
        return true;
    }

private:
    long N;
};

struct MonteCarloReduceFunctor : public PatternTree::MapFunctor<double*> {
    MonteCarloReduceFunctor(std::shared_ptr<PatternTree::View<double*>> temp) : temp(temp) {}

    void operator () (const int index, PatternTree::View<double*>& x) override {
        for (long i = 0; i < temp->shape()[0]; i++) {
            x(index) += (*temp)(i);
        }
    };

    void consumes(PatternTree::Dataflow& dataflow) override {
        dataflow.push_back(temp);
    };

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    {
        info.flops = temp->shape()[0];   
        return true;
    }

private:
    std::shared_ptr<PatternTree::View<double*>> temp;

};

std::unique_ptr<PatternTree::APT> monte_carlo(long draws, long estimates, int splits)
{    
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    PatternTree::APT::initialize(cluster, 1, estimates / splits, true);

    auto temp = PatternTree::APT::source<double*>("temp", estimates);

    for (int i = 0; i < splits; i++) {
        auto split = PatternTree::View<double*>::slice(temp->data(), std::make_pair(i * estimates / splits, (i + 1) * estimates / splits));

        std::unique_ptr<MonteCarloFunctor> functor(new MonteCarloFunctor(draws));    
        PatternTree::APT::map<double*, MonteCarloFunctor>("monte_carlo_map", std::move(functor), split);
    }

    auto res = PatternTree::APT::source<double*>("res", 1);
    std::unique_ptr<MonteCarloReduceFunctor> functor(new MonteCarloReduceFunctor(temp));    
    PatternTree::APT::map<double*, MonteCarloReduceFunctor>("monte_carlo_reduce", std::move(functor), res);

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    return std::move(apt);
};

class MonteCarloMappingBase : public PatternTree::IOptimizer {
public:
    MonteCarloMappingBase() {};

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

        PatternTree::IPattern& splitA = *it;
        step->assign(splitA, teamA_);

        if (size > 1)
        {
            PatternTree::IPattern& splitB = *(++it);
            step->assign(splitB, teamB_);   
        }
    };

    double costs() override {
        return -1.0;
    };

    std::shared_ptr<PatternTree::Team> teamA_;
    std::shared_ptr<PatternTree::Team> teamB_;
};

TEST(TestSuiteMonteCarlo, TestBase)
{
    long estimates = 96;
    long draws = 1000000000;
    auto apt = monte_carlo(draws, estimates, 2);

    ASSERT_EQ(apt->size(), 2);


    MonteCarloMappingBase mapping;
    apt->optimize(mapping);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    double runtime_ratio = runtime / 43.449;
    ASSERT_TRUE(runtime_ratio < 8 && runtime_ratio > 0.125);
}

class MonteCarloMappingDistributed : public PatternTree::IOptimizer {
public:
    MonteCarloMappingDistributed() {};

    void init(PatternTree::APT::Iterator begin, PatternTree::APT::Iterator end, const PatternTree::Cluster& cluster) override {
        std::shared_ptr<PatternTree::Node> nodeA = cluster.nodes().find("Node1")->second;

        std::shared_ptr<PatternTree::Device> deviceA = nodeA->devices().find("CPU1")->second;
        std::shared_ptr<PatternTree::Processor> processorA = deviceA->processors().find("1")->second;
        std::shared_ptr<PatternTree::Processor> processorB = deviceA->processors().find("2")->second;

        std::shared_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 24));
        std::shared_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 24));

        std::shared_ptr<PatternTree::Node> nodeB = cluster.nodes().find("Node2")->second;

        std::shared_ptr<PatternTree::Device> deviceB = nodeB->devices().find("CPU1")->second;
        std::shared_ptr<PatternTree::Processor> processorC = deviceB->processors().find("1")->second;
        std::shared_ptr<PatternTree::Processor> processorD = deviceB->processors().find("2")->second;

        std::shared_ptr<PatternTree::Team> teamC(new PatternTree::Team(processorC, 24));
        std::shared_ptr<PatternTree::Team> teamD(new PatternTree::Team(processorD, 24));

        this->teams = std::vector<std::shared_ptr<PatternTree::Team>>{teamA, teamB, teamC, teamD};
    };

    void assign(PatternTree::APT::Iterator& step) override {
        if (step->index() == 0) {
            auto team = this->teams.begin();
            for (auto it = step->begin(); it != step->end(); ++it) {
                step->assign(*it, *team);
                ++team;
            }
        } else {
            step->assign(*(step->begin()), teams[0]);
        }
    };

    double costs() override {
        return -1.0;
    };

    std::vector<std::shared_ptr<PatternTree::Team>> teams;
};

TEST(TestSuiteMonteCarlo, TestDistributed)
{
    long estimates = 96;
    long draws = 1000000000;
    auto apt = monte_carlo(draws, estimates, 4);

    ASSERT_EQ(apt->size(), 2);

    MonteCarloMappingDistributed mapping;
    apt->optimize(mapping);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    double runtime_ratio = runtime / 22.238;
    ASSERT_TRUE(runtime_ratio < 8 && runtime_ratio > 0.125);
}
