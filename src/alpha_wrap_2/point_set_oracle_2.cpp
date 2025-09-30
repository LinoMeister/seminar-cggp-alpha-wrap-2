#include "point_set_oracle_2.h"

namespace aw2 {

    bool point_set_oracle_2::empty() const { return tree_.empty(); }
    bool point_set_oracle_2::do_call() const { return (!empty()); }
    void point_set_oracle_2::clear() { tree_.clear(); }

    bool point_set_oracle_2::do_intersect(const Face_handle &t) const
    {
        if (tree_.empty()) return false;
        t->vertex(0)->point().x();

        // Build the bounding box of the triangle
        FT xmin = std::min({ t->vertex(0)->point().x(), t->vertex(1)->point().x(), t->vertex(2)->point().x() });
        FT xmax = std::max({ t->vertex(0)->point().x(), t->vertex(1)->point().x(), t->vertex(2)->point().x() });
        FT ymin = std::min({ t->vertex(0)->point().y(), t->vertex(1)->point().y(), t->vertex(2)->point().y() });
        FT ymax = std::max({ t->vertex(0)->point().y(), t->vertex(1)->point().y(), t->vertex(2)->point().y() });

        Point_2 pmin(xmin, ymin), pmax(xmax, ymax);

        // Fuzzy box query to restrict candidates
        CGAL::Fuzzy_iso_box<CGAL::Search_traits_2<K>> box(pmin, pmax);

        std::vector<Point_2> candidates;
        tree_.search(std::back_inserter(candidates), box);

        for (const auto &pt : candidates)
        {
            Point_2 tri_pts[3] = {
                t->vertex(0)->point(),
                t->vertex(1)->point(),
                t->vertex(2)->point()
            };
            if (CGAL::bounded_side_2(tri_pts, tri_pts + 3, pt, K()) != CGAL::ON_UNBOUNDED_SIDE)
            {
                // pt is inside or on the boundary
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

        Point_2 best;
        FT best_sqdist = std::numeric_limits<FT>::max();

        for (auto it = tree_.begin(); it != tree_.end(); ++it)
        {
            FT d2 = CGAL::squared_distance(*it, seg);
            if (d2 < best_sqdist)
            {
                best_sqdist = d2;
                best = *it;
            }
        }

        if (best_sqdist == std::numeric_limits<FT>::max())
            return false;

        if (std::sqrt(best_sqdist) > offset_size - intersection_precision)
            return false;


        // Compute the first intersection of the offset circle around best with the line segment p->q
        // There is probably a much nicer way to do this using CGAL's circular kernel... but for now:

        // Assumption: p is outside and q is potentially inside

        auto dx = q.x() - p.x();
        auto dy = q.y() - p.y();

        FT a = dx*dx + dy*dy;
        if (a == 0) return false; // degenerate segment

        // vector from circle center (best) to p
        FT ox = p.x() - best.x();
        FT oy = p.y() - best.y();

        FT b = 2 * (dx*ox + dy*oy);
        FT c = ox*ox + oy*oy - offset_size*offset_size;

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

}
