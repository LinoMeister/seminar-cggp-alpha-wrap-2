#ifndef AW2_TRAVERSABILITY_H
#define AW2_TRAVERSABILITY_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include <nlohmann/json.hpp>

namespace aw2 {

    // Forward declarations
    
    class point_set_oracle_2;
    using Oracle = point_set_oracle_2;

    struct Gate {
        Delaunay::Edge edge;
        FT priority;

        std::pair<Point_2, Point_2> get_points() const;
        std::pair<Delaunay::Vertex_handle, Delaunay::Vertex_handle> get_vertices() const;

        bool operator<(const Gate& other) const {
            return priority < other.priority;
        }
        
        bool operator>(const Gate& other) const {
            return priority > other.priority;
        }
    };

    using TraversabilityParams = std::variant<
        struct ConstantAlphaParams,
        struct AdaptiveAlphaParams,
        struct DistanceSamplingParams
    >;

    // Method-specific parameter structs
    struct ConstantAlphaParams {
        // No additional parameters for constant alpha
    };

    inline void to_json(nlohmann::json& j, const ConstantAlphaParams&) {
        j = nlohmann::json::object();
    }

    inline void from_json(const nlohmann::json&, ConstantAlphaParams&) {
        // No fields to populate
    }

    struct AdaptiveAlphaParams {
        FT alpha_max = 200.0;
        int point_threshold = 5;
        FT deviation_factor = 0.05;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AdaptiveAlphaParams, alpha_max, point_threshold, deviation_factor)
    };

    struct DistanceSamplingParams {
        FT tolerance_factor = 5.0;  // Multiplied by offset to get tolerance

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(DistanceSamplingParams, tolerance_factor)
    };

    // JSON serialization for TraversabilityParams variant
    inline void to_json(nlohmann::json& j, const TraversabilityParams& params) {
        std::visit([&j](const auto& p) {
            j = p;
        }, params);
    }

    inline void from_json(const nlohmann::json& j, TraversabilityParams& params) {
        // Note: This requires knowing which type to deserialize to
        // For now, we'll try each type and use the first one that works
        try {
            params = j.get<AdaptiveAlphaParams>();
        } catch (...) {
            try {
                params = j.get<DistanceSamplingParams>();
            } catch (...) {
                params = j.get<ConstantAlphaParams>();
            }
        }
    }

    class Traversability {
    public:
        virtual ~Traversability() = default;
        virtual bool operator()(Gate& g) = 0;
    };

    class ConstantAlphaTraversability : public Traversability {
    public:
        ConstantAlphaTraversability(FT alpha) : alpha_(alpha) {}

        virtual bool operator()(Gate& g) override {
            return g.priority >= alpha_;
        }

    private:
        FT alpha_;
    };

    class AdaptiveAlphaTraversability : public Traversability {
    public:
        AdaptiveAlphaTraversability(FT alpha, FT offset, const Oracle& oracle, AdaptiveAlphaParams params) 
            : alpha_(alpha), offset_(offset), oracle_(oracle),
              alpha_max_(params.alpha_max), point_threshold_(params.point_threshold), 
              deviation_factor_(params.deviation_factor) {}

        virtual bool operator()(Gate& g) override;

    private:
        FT subsegment_deviation(const Segment_2& seg) const;
        FT segment_deviation(const Segment_2& seg) const;
        FT alpha_;
        FT offset_;
        const Oracle& oracle_;
        FT alpha_max_;
        int point_threshold_;
        FT deviation_factor_;
    };

    class DistanceSamplingTraversability : public Traversability {
    public:
        DistanceSamplingTraversability(FT alpha, FT offset, const Oracle& oracle, DistanceSamplingParams params) 
            : alpha_(alpha), offset_(offset), oracle_(oracle),
              tolerance_(params.tolerance_factor * offset) {}

        virtual bool operator()(Gate& g) override;

    private:
        FT alpha_;
        FT offset_;
        const Oracle& oracle_;
        FT tolerance_;
    };

}

#endif // AW2_TRAVERSABILITY_H
