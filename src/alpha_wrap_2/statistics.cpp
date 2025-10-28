#include <alpha_wrap_2/statistics.h>
#include <chrono>
#include <ctime>

namespace aw2 {

std::string AlgorithmStatistics::to_json_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    
    oss << "{\n";
    
    // Config section
    oss << "  \"config\": {\n";
    oss << "    \"input_file\": \"" << config.input_file << "\",\n";
    oss << "    \"alpha\": " << config.alpha << ",\n";
    oss << "    \"offset\": " << config.offset << ",\n";
    oss << "    \"traversability_function\": \"" << config.traversability_function << "\",\n";
    oss << "    \"traversability_params\": " << (config.traversability_params.empty() ? "null" : config.traversability_params) << "\n";
    oss << "  },\n";
    
    // Output stats section
    oss << "  \"output_stats\": {\n";
    oss << "    \"n_vertices\": " << output_stats.n_vertices << ",\n";
    oss << "    \"n_edges\": " << output_stats.n_edges << "\n";
    oss << "  },\n";
    
    // Execution stats section
    oss << "  \"execution_stats\": {\n";
    oss << "    \"n_iterations\": " << execution_stats.n_iterations << ",\n";
    oss << "    \"n_rule_1\": " << execution_stats.n_rule_1 << ",\n";
    oss << "    \"n_rule_2\": " << execution_stats.n_rule_2 << "\n";
    oss << "  },\n";
    
    // Timings section
    oss << "  \"timings\": {\n";
    oss << "    \"total_time\": " << timings.total_time << ",\n";
    oss << "    \"gate_processing\": " << timings.gate_processing << ",\n";
    oss << "    \"rule_1_processing\": " << timings.rule_1_processing << ",\n";
    oss << "    \"rule_2_processing\": " << timings.rule_2_processing << "\n";
    oss << "  },\n";
    
    // Metadata section
    oss << "  \"metadata\": {\n";
    oss << "    \"is_test\": " << (metadata.is_test ? "true" : "false") << ",\n";
    oss << "    \"timestamp\": \"" << metadata.timestamp << "\",\n";
    oss << "    \"version\": \"" << metadata.version << "\"\n";
    oss << "  }\n";
    
    oss << "}";
    
    return oss.str();
}

void AlgorithmStatistics::export_to_json(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
    
    file << to_json_string();
    file.close();
}

} // namespace aw2
