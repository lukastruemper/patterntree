# PatternTree

![Build and tests](https://github.com/lukastruemper/patterntree/actions/workflows/cmake.yml/badge.svg)

PatternTree is a high-performance optimization framework for heterogeneous computing architectures, which allows to automatically transform and map parallel programs to heterogeneous architectures. A parallel program is thereby expressed as a computational graph, where the nodes of the graph are defined through *data* symbols and *parallel patterns* yielding the *Abstract Pattern Tree (APT)* [1]. The framework then optimizes the APT with respect to different *algorithmic efficiencies* automatically and maps it to the target architecture based on a static performance model and algorithmic properties [2]. In the next stage, the APT is executed on the target according to the mapping. Currently, PatternTree is **work in progress** and it will provide APIs for C++ and Python.

## Core Concepts

Basically, PatternTree is a high-level, data-centric parallel programming framework defining the flow of data through parallel patterns. This flow, or computational graph, is captured in the APT. The optimization part is based on the concept of algorithmic efficiencies, which define necessary optimality conditions for the APT. 

#### Data

PatternTree operates in two stages: In the first phase, in which the program is defined, data is a symbolic concept. This means, data is interpreted as a placeholder without a specific value. Operations and parallel patterns applied to this data are added to the APT. In the execution phase, an initial value must be provided for the data and the operations are now executed on the values as defined in the APT.

```c++
PatternTree::APT::init(cluster);

auto x = PatternTree::APT::data<double*>("x", 256);
x =  2 * x + 1;

std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

double* value = new double[256];
apt->execute({'x' : value});
```

#### Views

In general, data is accessed through views in a program. When defining new data, a full view on the data is returned. It is however often convenient to define subviews, when working on a subset of the data. Beside just facilitating the handling of data, subviews also allow for stronger assumptions in the optimizations regarding the parts of the data that need to be transferred between processors.

```c++
auto view = PatternTree::APT::data<double*>("x", 256);
auto subview = PatternTree::View<double*>::slice(view->data(), std::make_pair(32,96));
```

#### Parallel Patterns

Parallel patterns are a central concept of the modern parallel programming methodology and basically define specific structures of parallelism in computations. In PatternTree, parallelism must be expressed through specific parallel patterns such as the *map* or the *reduction*. This is done by writing a functor, which is passed to the higher-order function defined by the pattern. Defining parallelism through patterns improves code quality and it ensures a regularity facilitating the analysis of the program. 

```c++
struct IncrementFunctor : public PatternTree::MapFunctor<double*> {
	
    void operator () (int index, PatternTree::View<double*>& element) override {
        element = element + 1;
    };
};

auto x = PatternTree::APT::data<double*>("x", 256);

std::unique_ptr<MXVFunctor> functor(new IncrementFunctor());    
PatternTree::APT::map<double*, MXVFunctor>("increment", std::move(functor), x);
```

#### Algorithmic Efficiencies

Algorithmic efficiencies define necessary optimality conditions of performance over global properties of the APT. In this framework, two algorithmic efficiencies are considered:

**Synchronization Efficiency.** In a first stage, the dataflow between different parallel patterns is analyzed and parallelism between patterns is identified. In contrast to the local parallelism of the pattern itself, this parallelism defines a *global parallelism* of the APT. Maximizing this parallelism and pulling parallel patterns into earlier positions defines this efficiency. The state of the APT achieved by optimizing the efficiency can be seen as a normal-form before mapping the APT in the next stage.

**Inter-Processor Dataflow Efficiency.** In the second stage, the patterns of the APT are mapped to the processors of the target architecture. In order to determine an optimal mapping, the efficiency defines a cost for any mapping coresponding to an approximative runtime estimate. This cost depends on the mapping of other patterns through data dependencies and therefore provides a complex, global optimization criterion. Minimzing it with an optimizer yields transformations and mapping decisions similar to hand-tuned optimizations found in literature [2].

## Examples

#### Matrix-Vector Multiplication

```c++
struct MXVFunctor : public PatternTree::MapFunctor<double*> {
    
    MXVFunctor(std::shared_ptr<PatternTree::View<double**>> M,
        std::shared_ptr<PatternTree::View<double*>> x)
    : M_(M), x_(x) {}

    void operator () (int index, PatternTree::View<double*>& res) override {
        for (int j = 0; j < M_->shape()[1]; j++) {
            res += M_(i, j) * x_(j);
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
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("path");
    
    // BEGIN APT
    PatternTree::APT::init(cluster);

    // Declare data
    auto M = PatternTree::APT::data<double**>("M", 256, 256);
    auto x = PatternTree::APT::data<double*>("x", 256);
    auto res = PatternTree::APT::data<double*>("res", 256);

    // MXV as map pattern
    std::unique_ptr<MXVFunctor> functor(new MXVFunctor(M, x));    
    PatternTree::APT::map<double*, MXVFunctor>("mxv", std::move(functor), res);

    // END APT
    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    // Optimize and map
    PatternTree::StairClimbingOptimizer optimizer;
    apt->optimize(optimizer);

    // Evaluate estimated runtime
    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    // TODO: Execute
    // apt->execute()
}
```

#### Jacobi

```cpp
struct JacobiFunctor : public PatternTree::MapFunctor<double*> {
    JacobiFunctor(std::shared_ptr<PatternTree::View<double**>> A,
        std::shared_ptr<PatternTree::View<double*>> b,
        std::shared_ptr<PatternTree::View<double*>> x_)
    : A(A), b(b), x_(x_)
    {}

    void operator () (int index, PatternTree::View<double*>& x) override {
        x = b(index);
        for (size_t j = 0; j < N; j++)
        {
            if (j != index)
                x = x - A(index, j) * x_(j);
        }
        x = x / A(index, index);
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
    std::shared_ptr<PatternTree::Cluster> cluster = PatternTree::Cluster::parse("path");
    
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
        PatternTree::APT::map<double*, JacobiFunctor>("jacobi", std::move(functor), x);

        x.swap(x_);
    }

    std::unique_ptr<PatternTree::APT> apt = PatternTree::APT::compile();

    PatternTree::StairClimbingOptimizer optimizer;
    apt->optimize(optimizer);

    PatternTree::RooflineModel model;
    double runtime = apt->evaluate(model);

    // TODO: Execute
    // apt->execute()
}
```


## Roadmap

Version 1.0:

- [x] APT:
	- [x] Multidimensional data & views
	- [x] Map pattern
	  - [x] Instrumentation: FLOPS
	  - [x] Instrumentation: Bytes
	- [x] APT & steps
	- [x] Roofline model
	- [x] Basic mapping
	- [x] Synchronization Efficiency
	  - [x] Data splits
	  - [x] HappensBefore
	- [x] Global hyperparameter
	- [ ] Reduce pattern
	- [ ] Serial pattern
- [ ] Automatic mapping:
 	- [x] Index subviews
	- [ ] Pattern splits
	- [ ] StairClimbingOptimizer
	- [ ] Basic benchmarks

Extension:

- [ ] Functor reflection
- [ ] Lambda support
- [ ] Stencil pattern
- [ ] Data patterns: scatter & gather
- [ ] Recurrence patterns
- [ ] Distributed memory
- [ ] Python/Cython API:
    - [ ] Cython interface: https://cython.readthedocs.io/en/latest/src/userguide/memoryviews.html
	- [ ] Compatibilty with numpy API (NEP 18): https://numpy.org/neps/nep-0018-array-function-protocol.html

## References

1. J. Miller, L. Trümper, C. Terboven, M. S. Müller, Poster: *Efficiency of Algorithmic Structures*, in: IEEE/ACM International Conference on High Performance Computing, Networking, Storage and Analysis (SC19).
2. L. Trümper, J. Miller, C. Terboven, M. S. Müller (in-press). *Automatic Mapping of Parallel Pattern-based Algorithm on Heterogeneous Architectures*, in: Architecture of Computing Systems – ARCS 2021.
