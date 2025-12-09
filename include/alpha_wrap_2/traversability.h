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
        FT sq_min_delaunay_rad;

        std::pair<Point_2, Point_2> get_points() const;
        std::pair<Delaunay::Vertex_handle, Delaunay::Vertex_handle> get_vertices() const;

        bool operator<(const Gate& other) const {
            return sq_min_delaunay_rad < other.sq_min_delaunay_rad;
        }
        
        bool operator>(const Gate& other) const {
            return sq_min_delaunay_rad > other.sq_min_delaunay_rad;
        }
    };

    using TraversabilityParams = std::variant<
        struct ConstantAlphaParams,
        struct DeviationBasedParams,
        struct IntersectionBasedParams
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

    struct DeviationBasedParams {

        // The adaptive alpha is interpolated between alpha_ and alpha_max_ based on deviation
        FT alpha_max = 1.0;

        // Minimum number of points required to compute deviation
        // if fewer points are found, deviation is set to 1.0
        int point_threshold = 5;

        // Factor used to scale deviation to [0,1]. Higher values increase sensitivity, meaning deviation reaches 1.0 more quickly
        // and hence we end up with lower adaptive alpha values. => more gates are traversable.
        // This also implicitly defines the maximum deviation after which deviation is clamped to 1.0
        FT deviation_factor = 0.01;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(DeviationBasedParams, alpha_max, point_threshold, deviation_factor)
    };

    struct IntersectionBasedParams {
        
        // Multiplied by bbox diagonal length to get tolerance
        // A smaller tolerance means stricter traversability checks and potentially fewer traversable gates
        FT tolerance_factor = 0.005;  

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(IntersectionBasedParams, tolerance_factor)
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
            params = j.get<DeviationBasedParams>();
        } catch (...) {
            try {
                params = j.get<IntersectionBasedParams>();
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
            return g.sq_min_delaunay_rad >= alpha_ * alpha_;
        }

    private:
        FT alpha_;
    };

    class DeviationBasedTraversability : public Traversability {
    public:
        DeviationBasedTraversability(FT alpha, FT offset, FT bbox_diagonal_length, const Oracle& oracle, DeviationBasedParams params) 
            : alpha_(alpha), offset_(offset), oracle_(oracle),
              alpha_max_(params.alpha_max * bbox_diagonal_length), point_threshold_(params.point_threshold), 
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

    class IntersectionBasedTraversability : public Traversability {
    public:
        IntersectionBasedTraversability(FT alpha, FT offset, FT bbox_diagonal_length, const Oracle& oracle, IntersectionBasedParams params) 
            : alpha_(alpha), offset_(offset), oracle_(oracle),
              tolerance_(params.tolerance_factor * bbox_diagonal_length) {}

        virtual bool operator()(Gate& g) override;

    private:
        FT alpha_;
        FT offset_;
        const Oracle& oracle_;
        FT tolerance_;
    };

}

#endif // AW2_TRAVERSABILITY_H
