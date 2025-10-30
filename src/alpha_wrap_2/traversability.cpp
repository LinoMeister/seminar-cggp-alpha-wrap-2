#include "alpha_wrap_2/traversability.h"

namespace aw2 {

    std::pair<Delaunay::Vertex_handle, Delaunay::Vertex_handle> Gate::get_vertices() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second));
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second));
        return std::make_pair(v1, v2);
    }
    
    std::pair<Point_2, Point_2> Gate::get_points() const {
        auto v1 = edge.first->vertex(edge.first->cw(edge.second))->point();
        auto v2 = edge.first->vertex(edge.first->ccw(edge.second))->point();
        return std::make_pair(v1, v2);
    }

    // AdaptiveAlphaTraversability implementation
    bool AdaptiveAlphaTraversability::operator()(Gate& g) {
        auto points = g.get_points();
        Segment_2 seg(points.first, points.second);
        auto dev = segment_deviation(seg);
        auto adaptive_alpha = alpha_max_ * (1 - dev) + alpha_ * dev;
        return g.priority >= std::pow(adaptive_alpha, 2);
    }

    FT AdaptiveAlphaTraversability::subsegment_deviation(const Segment_2& seg) const {
        auto local_pts = oracle_.local_points(seg, offset_ + 4);
        int n = local_pts.size();

        // not enough points to compute a meaningful adaptive alpha
        if (n < point_threshold_) {
            return 1.0;
        }

        // compute average squared deviation from the segment
        auto avg_sq_deviation = 0.0;
        for (const auto& pt : local_pts) {
            avg_sq_deviation += CGAL::squared_distance(seg, pt);
        }

        avg_sq_deviation /= n;
        auto dev = deviation_factor_ * (avg_sq_deviation - std::pow(offset_, 2));
        dev = std::clamp(dev, 0.0, 1.0);

        return dev;
    }

    FT AdaptiveAlphaTraversability::segment_deviation(const Segment_2& seg) const {
        //auto segment_length = bbox_diagonal_length_ / 100.0;
        auto segment_length = alpha_;
        int m = std::ceil(std::sqrt(seg.squared_length()) / segment_length);
        auto s = seg.source();
        auto t = seg.target();

        auto max_dev = 0.0;
        for (int i = 0; i < m; ++i) {
            FT t0 = static_cast<FT>(i) / m;
            FT t1 = static_cast<FT>(i + 1) / m;
            auto p0 = s + t0 * (t - s);
            auto p1 = s + t1 * (t - s);
            Segment_2 sub_seg(p0, p1);
            auto dev = subsegment_deviation(sub_seg);
            if (dev > max_dev) {
                max_dev = dev;
            }
        }

        return std::clamp(max_dev, 0.0, 1.0);
    }

    // DistanceSamplingTraversability implementation
    bool DistanceSamplingTraversability::operator()(Gate& g) {
        auto points = g.get_points();
        Point_2 s = points.first;
        Point_2 t = points.second;


        CGAL::Line_2<K> line_corr(s,t);


        Segment_2 seg(s, t);
        auto segment_length = alpha_;
        int m = std::ceil(std::sqrt(seg.squared_length()) / segment_length);

        for (int i = 1; i < m; ++i) {
            FT t0 = static_cast<FT>(i) / m;
            Point_2 p0 = s + t0 * (t - s);

            auto perp = line_corr.perpendicular(p0).direction().to_vector();
            Point_2 p1 = p0 + 1000*perp;

            Point_2 steiner_point;
            FT lambda;
            bool intersects = oracle_.first_intersection(
                p0,
                p1,
                steiner_point,
                offset_,
                lambda
            );
            if (intersects && CGAL::squared_distance(p0, steiner_point) > std::pow(tolerance_factor_ * offset_, 2)) {
                return true;
            }
            else if (!intersects) {
                return true;
            }
        }
        return false;
    }


}
