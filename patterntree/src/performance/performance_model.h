#pragma once

#include <nlohmann/json.hpp>

namespace PatternTree
{

class Step;

class IPerformanceModel {
public:
        static constexpr double FREQUENCY_TO_SECONDS = 1e6; // Megahertz = 1e6.

        static constexpr double LATENCY_TO_SECONDS = 1e9; // Nanoseconds = 1e9.

        static constexpr double BANDWIDTH_TO_SECONDS = 1e3; // bandwidth in megabyte/s = 1e6, sizes in kilobytes,

        /**
         * Estimated costs for current state.
         * 
         * @return costs
         */
        virtual double costs() = 0;

        /**
         * Updates state of cost estimation with next step.
         *
         * @param step
         */
        virtual void update(Step& step) = 0;

        /**
         * Report of cost estimation as json.
         * 
         * @return nlohmann::json
         */
        virtual nlohmann::json report() = 0;
};
}
