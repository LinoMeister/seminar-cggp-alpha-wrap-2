#ifndef AW2_STATISTICS_H
#define AW2_STATISTICS_H

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "alpha_wrap_2/traversability.h"

namespace aw2 {

struct TimingStats {
    double total_time = 0.0;
    double gate_processing = 0.0;
    double rule_1_processing = 0.0;
    double rule_2_processing = 0.0;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimingStats, total_time, gate_processing, rule_1_processing, rule_2_processing)
};

struct ExecutionStats {
    int n_iterations = 0;
    int n_rule_1 = 0;
    int n_rule_2 = 0;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExecutionStats, n_iterations, n_rule_1, n_rule_2)
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

struct MetadataStats {
    bool is_test = true; // set to false for 'real' experiments
    std::string timestamp;
    std::string version = "1.0";
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MetadataStats, is_test, timestamp, version)
};

struct AlgorithmStatistics {
    ConfigStats config;
    OutputStats output_stats;
    ExecutionStats execution_stats;
    TimingStats timings;
    MetadataStats metadata;

    // Export to JSON file
    void export_to_json(const std::string& filepath) const;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AlgorithmStatistics, config, output_stats, execution_stats, timings, metadata)
};

} // namespace aw2

#endif // AW2_STATISTICS_H
