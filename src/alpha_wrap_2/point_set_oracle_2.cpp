#include <alpha_wrap_2/point_set_oracle_2.h>

namespace aw2 {

    // Forward declaration of segment_circle_intersection
    bool segment_circle_intersection(const Point_2 &p, const Point_2 &q, const Point_2 &center, FT radius, Point_2 &o);

    bool point_set_oracle_2::empty() const { return tree_.empty(); }
    bool point_set_oracle_2::do_call() const { return (!empty()); }
    void point_set_oracle_2::clear() { tree_.clear(); }

    bool point_set_oracle_2::do_intersect(const K::Triangle_2 &t) const
    {
        if (tree_.empty()) return false;
        auto bbox = t.bbox();
        auto min = Point_2(bbox.xmin(), bbox.ymin());
        auto max = Point_2(bbox.xmax(), bbox.ymax());
        
        // Fuzzy box query to restrict candidates
        CGAL::Fuzzy_iso_box<CGAL::Search_traits_2<K>> box(min, max);

        std::vector<Point_2> candidates;
        tree_.search(std::back_inserter(candidates), box);

        for (const auto &pt : candidates)
        {
            if (t.has_on_bounded_side(pt)) {
                return true;
            }
        }
        return false;
    }

    FT point_set_oracle_2::squared_distance(const Point_2 &p) const
    {
        if (tree_.empty()) return 0.0;

        Neighbor_search search(tree_, p, 1);
        return search.begin()->second; // squared distance to nearest neighbor
    }

    Point_2 point_set_oracle_2::closest_point(const Point_2 &p) const
    {
        if (tree_.empty()) return Point_2(0, 0);

        Neighbor_search search(tree_, p, 1);
        return search.begin()->first; // nearest point
    }

    bool point_set_oracle_2::first_intersection(const Point_2 &p, const Point_2 &q,
                            Point_2 &o,
                            const FT offset_size,
                            const FT intersection_precision) const
    {
        if (tree_.empty()) return false;
        Segment_2 seg(p, q);

        // get a bounding box with margins so we narrow down the search
        CGAL::Bbox_2 bbox = seg.bbox();
        Point_2 min(bbox.xmin() - offset_size, bbox.ymin() - offset_size);
        Point_2 max(bbox.xmax() + offset_size, bbox.ymax() + offset_size);
        CGAL::Fuzzy_iso_box<Traits> box(min, max);

        std::vector<Point_2> candidates;
        tree_.search(std::back_inserter(candidates), box);

        if (candidates.empty()) {
            return false;
        }

        // Create a temporary kd-tree from candidates for efficient proximity search
        Tree candidates_tree(candidates.begin(), candidates.end());
        Incremental_neighbor_search inc_search(candidates_tree, p);

        
        // Iterate through candidates by proximity to p
        for (auto it = inc_search.begin(); it != inc_search.end(); ++it)
        {
            if (segment_circle_intersection(p, q, it->first, offset_size, o)) {
                return true;
            }
        }
        return false;
    }

    bool point_set_oracle_2::first_intersection(const Point_2 &p, const Point_2 &q,
                            Point_2 &o,
                            const FT offset_size) const
    {
        return first_intersection(p, q, o, offset_size, 0.1);
    }

    void point_set_oracle_2::add_point_set(const Points& points) {
        tree_.insert(points.cbegin(), points.cend());
    }

    void point_set_oracle_2::load_points(const std::string& filename) {
        auto points = std::vector<Point_2>();
        std::ifstream input(filename);
        double x, y;
        while (input >> x >> y) {
            points.emplace_back(x, y);

            if (x < bbox_.x_min)
                bbox_.x_min = x;
            if (x > bbox_.x_max)
                bbox_.x_max = x;
            if (y < bbox_.y_min)
                bbox_.y_min = y;
            if (y > bbox_.y_max)
                bbox_.y_max = y;
        }

        add_point_set(points);
    }

    Points point_set_oracle_2::local_points(const Segment_2 &seg, const FT margin) const {
        Points local_pts;
        if (tree_.empty()) return local_pts;

        auto bbox = seg.bbox();
        Point_2 min(bbox.xmin() - margin, bbox.ymin() - margin);
        Point_2 max(bbox.xmax() + margin, bbox.ymax() + margin);
        CGAL::Fuzzy_iso_box<Traits> box(min, max);

        // Search for points within the bounding box
        tree_.search(std::back_inserter(local_pts), box);
        return local_pts;
    }

    bool segment_circle_intersection(const Point_2 &p, const Point_2 &q, const Point_2 &center, FT radius, Point_2 &o) {
        auto dx = q.x() - p.x();
        auto dy = q.y() - p.y();

        FT a = dx*dx + dy*dy;
        if (a == 0) return false; // degenerate segment

        FT ox = p.x() - center.x();
        FT oy = p.y() - center.y();

        FT b = 2 * (dx*ox + dy*oy);
        FT c = ox*ox + oy*oy - radius*radius;

        FT disc = b*b - 4*a*c;
        if (disc < 0) return false; // no real intersection

        FT sqrt_disc = std::sqrt(disc);
        FT t1 = (-b - sqrt_disc) / (2*a);
        FT t2 = (-b + sqrt_disc) / (2*a);

        FT t_candidate = std::numeric_limits<FT>::max();
        if (t1 > 0.0 && t1 <= 1.0) t_candidate = std::min(t_candidate, t1);
        if (t2 > 0.0 && t2 <= 1.0) t_candidate = std::min(t_candidate, t2);

        if (t_candidate == std::numeric_limits<FT>::max())
            return false;

        // compute intersection point
        o = Point_2(p.x() + t_candidate*dx, p.y() + t_candidate*dy);
        return true;
    }

}
