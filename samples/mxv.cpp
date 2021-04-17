#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

struct MXVFunctor : public PatternTree::MapFunctor<double*> {
    
    MXVFunctor(std::shared_ptr<PatternTree::View<double**>> M, std::shared_ptr<PatternTree::View<double*>> x)
    : M_(M), x_(x) {}

    void operator () (int index, PatternTree::View<double*>& res) override {
        for (int j = 0; j < M_->shape()[1]; j++) {
            res = res + (*M_)(index, j) * (*x_)(j);
        }
    };

    void consumes(PatternTree::Dataflow& dataflow) override {
        // Declare additional data to be read by the pattern
        // TODO: Automatically scan members with reflection technique
		dataflow.push_back(M_);
        dataflow.push_back(x_);
    };

private:
    std::shared_ptr<PatternTree::View<double**>> M_;
    std::shared_ptr<PatternTree::View<double*>> x_;
};

int main() {
	std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    
	// BEGIN APT
	PatternTree::APT::initialize(cluster);

	// Declare data
    auto M = PatternTree::APT::source<double**>("M", 256, 256);
    auto x = PatternTree::APT::source<double*>("x", 256);
    auto res = PatternTree::APT::source<double*>("res", 256);

	// MXV as map pattern
    std::unique_ptr<MXVFunctor> functor(new MXVFunctor(M, x));    
    PatternTree::APT::map<double*, MXVFunctor>(std::move(functor), res);

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