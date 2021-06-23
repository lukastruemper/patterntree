#include <apt/apt.h>
#include <apt/step.h>
#include <patterns/map.h>

#include <cluster/cluster.h>
#include <cluster/team.h>

#include <optimization/optimizer.h>

#include <performance/dataflow_state.h>
#include <performance/roofline_model.h>

struct MandelbrotFunctor : public PatternTree::MapFunctor<double*> {
    MandelbrotFunctor(int dim0)
    {
        this->dim0 = dim0;
    }

    void operator () (int index, PatternTree::View<double*>& element) override {
        double x0 = ((index % dim0) / ((double) dim0) * 3.5) - 1.0;
        double y0 = ((index / ((double) dim0)) / ((double) dim0) * 2.0) - 1.0;
        double x = 0.0;
        double y = 0.0;
    
        int iteration = 0;
        int max_iteration = 100;
        while (x*x + y*y <= 2*2 && iteration < max_iteration)
        {
            double xtemp = x*x - y*y + x0;
            y = 2*x*y + y0;
            x = xtemp;

            iteration++;
            element = element + 1.0;
        }
    };

    void consumes(PatternTree::Dataflow& dataflow) override {};

    bool touch(const int index, PatternTree::PatternIndexInfo &info) override
    {
        double x0 = ((index % dim0) / ((double) dim0) * 3.5) - 1.0;
        double y0 = ((index / ((double) dim0)) / ((double) dim0) * 2.0) - 1.0;
        double x = 0.0;
        double y = 0.0;
    
        int iteration = 0;
        int max_iteration = 100;

        info.flops = 0;
        while (x*x + y*y <= 2*2 && iteration < max_iteration)
        {
            info.flops += 7;

            double xtemp = x*x - y*y + x0;
            y = 2*x*y + y0;
            x = xtemp;

            iteration++;

            info.flops += 8;
        }
        return true;
    }

    private:
        int dim0;
};


int main() {
    size_t N = 1024;

	std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("../clusters/cluster_c18g.json");    
    
    PatternTree::APT::init(cluster);

    auto set = PatternTree::APT::data<double*>("set", N * N);

    std::unique_ptr<MandelbrotFunctor> functor(new MandelbrotFunctor(N));    
    PatternTree::APT::map<double*, MandelbrotFunctor>(std::move(functor), set);

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
