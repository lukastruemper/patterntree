#pragma once

#include <map>

#include "apt/step.h"
#include "performance/dataflow_state.h"
#include "performance/performance_model.h"

namespace PatternTree
{
class RooflineModel : public IPerformanceModel {
    DataflowState state_;

    double current_costs_;
    std::vector<std::unordered_map<const Team*, std::pair<double, double>>> costs_;
    std::vector<std::pair<double, double>> max_costs_;

    static constexpr double ROOFLINE_OVERLAP = 0.0;
public:

    RooflineModel();

    double costs() override;

    void update(Step& step) override;

    nlohmann::json report() override;

    /**
     * Estimates the execution costs of the splits with the team.
     *
     * @param splits
     * @param team
     * @return costs
     */
    double execution_costs(const std::vector<std::reference_wrapper<const PatternSplit>>& splits, const Team& team);

    /**
     * Estimates the network costs of the splits with the team.
     *
     * @param splits
     * @param team
     * @return costs
     */
    double network_costs(const std::vector<std::reference_wrapper<const PatternSplit>>& splits, const Team& team);

};
}
