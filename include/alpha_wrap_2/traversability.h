#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"

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
        AdaptiveAlphaTraversability(FT alpha, FT offset, const Oracle& oracle) : alpha_(alpha), offset_(offset), oracle_(oracle) {}

        virtual bool operator()(Gate& g) override;

    private:
        FT subsegment_deviation(const Segment_2& seg) const;
        FT segment_deviation(const Segment_2& seg) const;
        FT alpha_;
        FT alpha_max_ = 200; // TODO make configurable
        FT offset_;
        int point_threshold_ = 5;
        FT deviation_factor_ = 0.05;
        const Oracle& oracle_;
    };

    class DistanceSamplingTraversability : public Traversability {
    public:
        DistanceSamplingTraversability(FT alpha, FT offset, const Oracle& oracle) : alpha_(alpha), offset_(offset), oracle_(oracle) {}

        virtual bool operator()(Gate& g) override;

    private:
        FT alpha_;
        FT offset_;
        FT tolerance_factor_ = 5.0;
        const Oracle& oracle_;
    };

}

