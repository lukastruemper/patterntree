#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

#include <data/data_concepts.h>

struct KMeansAssignFunctor : public PatternTree::MapFunctor<int*> {

    KMeansAssignFunctor(std::shared_ptr<PatternTree::View<double**>> points, std::shared_ptr<PatternTree::View<double**>> centroids)
    : points_(points), centroids_(centroids)
    {}

    void operator () (int index, PatternTree::View<int*>& assignment) override {
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

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    { 
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

    void operator () (int k, PatternTree::View<double**>& centroid) override {
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

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    {
        int N = points_->shape()[0];
        
        info.flops = N * (2 + 1 + 1 + 2) + 2;

        return true;
    }

private:
size_t K_;
std::shared_ptr<PatternTree::View<double**>> points_;
std::shared_ptr<PatternTree::View<int*>> assignment_;

};

int main() {
	size_t niters = 100;
    size_t K = 6;
    size_t N = 256;
    
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    
	// BEGIN APT
	PatternTree::APT::initialize(cluster);

    auto points = PatternTree::APT::source<double**>("points", N, 2);
    auto centroids = PatternTree::APT::source<double**>("centroids", K, 2);
    auto assignment = PatternTree::APT::source<int*>("assignment", N);

    for (size_t i = 0; i < niters; i++) {
        std::unique_ptr<KMeansAssignFunctor> assign_functor(new KMeansAssignFunctor(points, centroids));    
        PatternTree::APT::map<int*, KMeansAssignFunctor>(std::move(assign_functor), assignment);

        std::unique_ptr<KMeansUpdateFunctor> update_functor(new KMeansUpdateFunctor(K, points, assignment));    
        PatternTree::APT::map<double**, KMeansUpdateFunctor>(std::move(update_functor), centroids);
    }

	// END APT
    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

	// Optimize and map
    //PatternTree::StairClimbingOptimizer optimizer;
    //apt->optimize(optimizer);

	// Evaluate estimated runtime
    //PatternTree::RooflineModel model;
    //double runtime = apt->evaluate(model);

	// TODO: Execute
	// apt->execute()
}