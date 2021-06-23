#pragma once

#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

struct KMeansAssignFunctor : public PatternTree::MapFunctor<int*> {

    KMeansAssignFunctor(std::shared_ptr<PatternTree::View<double**>> points, std::shared_ptr<PatternTree::View<double**>> centroids)
    : points_(points), centroids_(centroids)
    {}

    void operator () (const int index, PatternTree::View<int*>& assignment) override {
        double optimal_dist = DBL_MAX;
        for (int k = 0; k < centroids_->shape()[0]; ++k) {
            double x_dist = (*points_)(index, 0) - (*centroids_)(index, 0);
            double y_dist = (*points_)(index, 1) - (*centroids_)(index, 1);
            double dist = x_dist * x_dist + y_dist * y_dist;
            if (dist < optimal_dist) {
                optimal_dist = dist;
                assignment = k;
            }
        }
    }

    void consumes(PatternTree::Dataflow& dataflow) override {
        dataflow.push_back(points_);
        dataflow.push_back(centroids_);
    }

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override { 
        info.flops = 6 * centroids_->shape()[0];

        return true;
    }

private:
std::shared_ptr<PatternTree::View<double**>> points_;
std::shared_ptr<PatternTree::View<double**>> centroids_;

};

struct KMeansUpdateFunctor : public PatternTree::MapFunctor<double**> {

    KMeansUpdateFunctor(size_t K, std::shared_ptr<PatternTree::View<double**>> points, std::shared_ptr<PatternTree::View<int*>> assignment)
    : K_(K), points_(points), assignment_(assignment)
    {}

    void operator () (const int k, PatternTree::View<double**>& centroid) override {
        int count = 0;
        double sum_x = 0.;
        double sum_y = 0.;
        for (int i = 0; i < points_->shape()[0]; ++i) {
            if ((*assignment_)(i) == k) {
                count++;
                sum_x += (*points_)(i, 0);
                sum_y += (*points_)(i, 1);
            }
        }
        if (count != 0) {
            centroid(k, 0) = sum_x / count;
            centroid(k, 1) = sum_y / count;
        }
    }

    void consumes(PatternTree::Dataflow& dataflow) override {
        dataflow.push_back(points_);
        dataflow.push_back(assignment_);
    }

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override { 
        int N = points_->shape()[0];
        info.flops = N * (2 + 1 + 1 + 2) + 2;
        
        return true;
    }

private:
size_t K_;
std::shared_ptr<PatternTree::View<double**>> points_;
std::shared_ptr<PatternTree::View<int*>> assignment_;

};

class KMeansMappingCPU : public PatternTree::IOptimizer {
public:
    KMeansMappingCPU() {};

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

        step->assign(*it, teamA_);
        step->assign(*(++it), teamB_);
    };

    double costs() override {
        return -1.0;
    };

    std::shared_ptr<PatternTree::Team> teamA_;
    std::shared_ptr<PatternTree::Team> teamB_;
};

class KMeansMappingGPU : public PatternTree::IOptimizer {
public:
    KMeansMappingGPU() {};

    void init(PatternTree::APT::Iterator begin, PatternTree::APT::Iterator end, const PatternTree::Cluster& cluster) override {
        std::shared_ptr<PatternTree::Node> node = cluster.nodes().begin()->second;
        std::shared_ptr<PatternTree::Device> device = node->devices().find("GPU1")->second;
        std::shared_ptr<PatternTree::Processor> processorA = (device->processors().begin())->second;
        std::shared_ptr<PatternTree::Processor> processorB = (++(device->processors().begin()))->second;

        std::unique_ptr<PatternTree::Team> teamA(new PatternTree::Team(processorA, 2560));
        std::unique_ptr<PatternTree::Team> teamB(new PatternTree::Team(processorB, 2560));
        teamA_ = std::move(teamA);
        teamB_ = std::move(teamB);
    };

    void assign(PatternTree::APT::Iterator& step) override {
        auto it = step->begin();

        step->assign(*it, teamA_);
        step->assign(*(++it), teamB_);
    };

    double costs() override {
        return -1.0;
    };

    std::shared_ptr<PatternTree::Team> teamA_;
    std::shared_ptr<PatternTree::Team> teamB_;
};

TEST(TestSuiteKMeans, TestCPU)
{
    int K = 128;
    int N = 10000000;
    int niters = 100;

    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g_simple.json");
    
	// BEGIN APT
	PatternTree::APT::initialize(cluster, 2, 2);

    auto points = PatternTree::APT::source<double**>("points", N, 2);
    auto centroids = PatternTree::APT::source<double**>("centroids", K, 2);
    auto assignment = PatternTree::APT::source<int*>("assignment", N);

    auto points_A = PatternTree::View<double**>::slice(points->data(), std::make_pair(0, N / 2), std::make_pair(0, 2));
    auto points_B = PatternTree::View<double**>::slice(points->data(), std::make_pair(N / 2, N), std::make_pair(0, 2));
    auto assignment_A = PatternTree::View<int*>::slice(assignment->data(), std::make_pair(0, N / 2));
    auto assignment_B = PatternTree::View<int*>::slice(assignment->data(), std::make_pair(N / 2, N));

    auto centroids_A = PatternTree::View<double**>::slice(centroids->data(), std::make_pair(0, K / 2), std::make_pair(0, 2));
    auto centroids_B = PatternTree::View<double**>::slice(centroids->data(), std::make_pair(K / 2, K), std::make_pair(0, 2));

    for (size_t i = 0; i < niters; i++) {

        std::unique_ptr<KMeansAssignFunctor> assign_functor_A(new KMeansAssignFunctor(points_A, centroids));    
        PatternTree::APT::map<int*, KMeansAssignFunctor>("kmeans_assign_lower", std::move(assign_functor_A), assignment_A);

        std::unique_ptr<KMeansAssignFunctor> assign_functor_B(new KMeansAssignFunctor(points_B, centroids));    
        PatternTree::APT::map<int*, KMeansAssignFunctor>("kmeans_assign_upper", std::move(assign_functor_B), assignment_B);

        std::unique_ptr<KMeansUpdateFunctor> update_functor_A(new KMeansUpdateFunctor(K, points, assignment));    
        PatternTree::APT::map<double**, KMeansUpdateFunctor>("kmeans_update_lower", std::move(update_functor_A), centroids_A);

        std::unique_ptr<KMeansUpdateFunctor> update_functor_B(new KMeansUpdateFunctor(K, points, assignment));    
        PatternTree::APT::map<double**, KMeansUpdateFunctor>("kmeans_update_upper", std::move(update_functor_B), centroids_B);
    }

	// END APT
    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    KMeansMappingCPU mapping;
    apt->optimize(mapping);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    double runtime_ratio = runtime / 9.594;
    ASSERT_TRUE(runtime_ratio < 2 && runtime_ratio > 0.5);
}

TEST(TestSuiteKMeans, TestGPU)
{
    int K = 128;
    int N = 10000000;
    int niters = 100;

    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g_simple.json");
    
	// BEGIN APT
	PatternTree::APT::initialize(cluster, 2, 2);

    auto points = PatternTree::APT::source<double**>("points", N, 2);
    auto centroids = PatternTree::APT::source<double**>("centroids", K, 2);
    auto assignment = PatternTree::APT::source<int*>("assignment", N);

    auto points_A = PatternTree::View<double**>::slice(points->data(), std::make_pair(0, N / 2), std::make_pair(0, 2));
    auto points_B = PatternTree::View<double**>::slice(points->data(), std::make_pair(N / 2, N), std::make_pair(0, 2));
    auto assignment_A = PatternTree::View<int*>::slice(assignment->data(), std::make_pair(0, N / 2));
    auto assignment_B = PatternTree::View<int*>::slice(assignment->data(), std::make_pair(N / 2, N));

    auto centroids_A = PatternTree::View<double**>::slice(centroids->data(), std::make_pair(0, K / 2), std::make_pair(0, 2));
    auto centroids_B = PatternTree::View<double**>::slice(centroids->data(), std::make_pair(K / 2, K), std::make_pair(0, 2));

    for (size_t i = 0; i < niters; i++) {

        std::unique_ptr<KMeansAssignFunctor> assign_functor_A(new KMeansAssignFunctor(points_A, centroids));    
        PatternTree::APT::map<int*, KMeansAssignFunctor>("kmeans_assign_lower", std::move(assign_functor_A), assignment_A);

        std::unique_ptr<KMeansAssignFunctor> assign_functor_B(new KMeansAssignFunctor(points_B, centroids));    
        PatternTree::APT::map<int*, KMeansAssignFunctor>("kmeans_assign_upper", std::move(assign_functor_B), assignment_B);

        std::unique_ptr<KMeansUpdateFunctor> update_functor_A(new KMeansUpdateFunctor(K, points, assignment));    
        PatternTree::APT::map<double**, KMeansUpdateFunctor>("kmeans_update_lower", std::move(update_functor_A), centroids_A);

        std::unique_ptr<KMeansUpdateFunctor> update_functor_B(new KMeansUpdateFunctor(K, points, assignment));    
        PatternTree::APT::map<double**, KMeansUpdateFunctor>("kmeans_update_upper", std::move(update_functor_B), centroids_B);
    }

	// END APT
    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    KMeansMappingGPU mapping;
    apt->optimize(mapping);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    double runtime_ratio = runtime / 3.921;
    ASSERT_TRUE(runtime_ratio < 2 && runtime_ratio > 0.5);
}
