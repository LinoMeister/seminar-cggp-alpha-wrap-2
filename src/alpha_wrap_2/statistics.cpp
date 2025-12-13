#include <alpha_wrap_2/statistics.h>

namespace aw2 {
    using json = nlohmann::json;

    void AlgorithmStatistics::export_to_json(const std::string &filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + filepath);
        }

        const json j = *this;
        file << j.dump(2);
        file.close();
    }
} // namespace aw2
