#pragma once

namespace PatternTree
{

class Step;

class IPerformanceModel {
public:
        static constexpr double FREQUENCY_TO_SECONDS = 1e6; // Megahertz = 1e6.

        static constexpr double LATENCY_TO_SECONDS = 1e9; // Nanoseconds = 1e9.

        static constexpr double BANDWIDTH_TO_SECONDS = 1e3; // bandwidth in megabyte/s = 1e6, sizes in kilobytes,

        virtual void update(Step& step) = 0;
        
        virtual double runtime() = 0;
};
}
