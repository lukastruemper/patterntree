#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

struct JacobiFunctor : public PatternTree::MapFunctor<double*> {
    JacobiFunctor(std::shared_ptr<PatternTree::View<double**>> A, std::shared_ptr<PatternTree::View<double*>> b, std::shared_ptr<PatternTree::View<double*>> x_)
    : A(A), b(b), x_(x_)
    {}

    void operator () (int index, PatternTree::View<double*>& x) override {
        size_t N = A->shape()[0];
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
        int N = A->shape()[0];
        
        // Loop: Check & increment
        info.flops = 2 * N;
        // Check i != j and update x
        info.flops += N * 3;
        // Divison
        info.flops += 1;

        return true;
    }

private:
    std::shared_ptr<PatternTree::View<double**>> A;
    std::shared_ptr<PatternTree::View<double*>> b;
    std::shared_ptr<PatternTree::View<double*>> x_;
};

int main() {
	std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");
    
	// BEGIN APT
	PatternTree::APT::init(cluster);

    int K = 50;
    int N = 8192;

    auto A = PatternTree::APT::data<double**>("A", N, N);
    auto b = PatternTree::APT::data<double*>("b", N);
    auto x = PatternTree::APT::data<double*>("x", N);
    auto x_ = PatternTree::APT::data<double*>("x_", N);

    for (int k = 0; k < K; k++)
    {
        std::unique_ptr<JacobiFunctor> functor(new JacobiFunctor(A, b, x_));    
        PatternTree::APT::map<double*, JacobiFunctor>(std::move(functor), x);

        x.swap(x_);
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