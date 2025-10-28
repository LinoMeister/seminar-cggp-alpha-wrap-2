#ifndef AW2_STATISTICS_H
#define AW2_STATISTICS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace aw2 {

struct TimingStats {
    double total_time = 0.0;
    double gate_processing = 0.0;
    double rule_1_processing = 0.0;
    double rule_2_processing = 0.0;
};

struct ExecutionStats {
    int n_iterations = 0;
    int n_rule_1 = 0;
    int n_rule_2 = 0;
};

struct OutputStats {
    int n_vertices = 0;
    int n_edges = 0;
};

struct ConfigStats {
    std::string input_file;
    double alpha = 0.0;
    double offset = 0.0;
    std::string traversability_function;
    std::string traversability_params; // JSON string for additional params
};

struct MetadataStats {
    bool is_test = false;
    std::string timestamp;
    std::string version = "1.0";
};

struct AlgorithmStatistics {
    ConfigStats config;
    OutputStats output_stats;
    ExecutionStats execution_stats;
    TimingStats timings;
    MetadataStats metadata;

    // Export to JSON file
    void export_to_json(const std::string& filepath) const;
    
    // Convert to JSON string
    std::string to_json_string() const;
};

} // namespace aw2

#endif // AW2_STATISTICS_H
