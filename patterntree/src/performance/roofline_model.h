#pragma once

#include "apt/step.h"
#include "performance/dataflow_state.h"
#include "performance/performance_model.h"

namespace PatternTree
{
class RooflineModel : public IPerformanceModel {
    double runtime_;
    DataflowState state_;
    std::vector<std::pair<double, double>> costs_;

public:
    static constexpr double ROOFLINE_OVERLAP = 0.0;

    RooflineModel() : runtime_(0), state_(), costs_() {};

    double execution_costs(const std::vector<std::reference_wrapper<const PatternSplit>>& splits, const Team& team);
    double network_costs(const std::vector<std::reference_wrapper<const PatternSplit>>& splits, const Team& team);

    void update(Step& step) override;
    double runtime() override;

    std::pair<double, double> costs_at(size_t index);
};
}
