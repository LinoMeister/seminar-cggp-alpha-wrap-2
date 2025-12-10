#ifndef AW2_STATISTICS_H
#define AW2_STATISTICS_H

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "alpha_wrap_2/traversability.h"

namespace aw2 {

struct TimingStats {
    double total_time = 0.0;
    double main_loop = 0.0;
    double gate_processing = 0.0;
    double rule_1_processing = 0.0;
    double rule_2_processing = 0.0;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimingStats, total_time, main_loop, gate_processing, rule_1_processing, rule_2_processing)
};

struct ExecutionStats {
    int n_iterations = 0;
    int n_rule_1 = 0;
    int n_rule_2 = 0;
    int n_input_points = 0;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExecutionStats, n_iterations, n_rule_1, n_rule_2, n_input_points)
};

struct OutputStats {
    int n_vertices = 0;
    int n_edges = 0;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(OutputStats, n_vertices, n_edges)
};

struct ConfigStats {
    std::string input_file;
    double alpha = 0.0;
    double offset = 0.0;
    std::string traversability_function;
    TraversabilityParams traversability_params;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConfigStats, input_file, alpha, offset, traversability_function, traversability_params)
};

struct AlgorithmStatistics {
    ConfigStats config;
    OutputStats output_stats;
    ExecutionStats execution_stats;
    TimingStats timings;

    // Export to JSON file
    void export_to_json(const std::string& filepath) const;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AlgorithmStatistics, config, output_stats, execution_stats, timings)
};

} // namespace aw2

#endif // AW2_STATISTICS_H
