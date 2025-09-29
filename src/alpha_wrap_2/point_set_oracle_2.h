#include <CGAL/Default.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Kd_tree.h>
#include <CGAL/Kd_tree_rectangle.h>
#include <CGAL/Search_traits_2.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Fuzzy_iso_box.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/Exact_circular_kernel_2.h>
#include "types.h"


namespace aw2 {

struct bbox_2 {
    FT x_min = std::numeric_limits<FT>::infinity();
    FT x_max = -std::numeric_limits<FT>::infinity();
    FT y_min = std::numeric_limits<FT>::infinity();
    FT y_max = -std::numeric_limits<FT>::infinity();
};

class point_set_oracle_2
{
protected:    

    using Traits = CGAL::Search_traits_2<K>;
    using Tree = CGAL::Kd_tree<Traits>;
    using BBox = bbox_2;
    typedef CGAL::Orthogonal_k_neighbor_search<Traits> Neighbor_search;


public:

    bool empty() const { return tree_.empty(); }
    bool do_call() const { return (!empty()); }
    void clear() { tree_.clear(); }

    bool do_intersect(const Face_handle &t) const
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

    FT squared_distance(const Point_2 &p) const
    {
        if (tree_.empty()) return 0.0;

        Neighbor_search search(tree_, p, 1);
        return search.begin()->second; // squared distance to nearest neighbor
    }

    Point_2 closest_point(const Point_2 &p) const
    {
        if (tree_.empty()) return Point_2(0, 0);

        Neighbor_search search(tree_, p, 1);
        return search.begin()->first; // nearest point
    }

    bool first_intersection(const Point_2 &p, const Point_2 &q,
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

        o = best;
        return true;
    }

    bool first_intersection(const Point_2 &p, const Point_2 &q,
                            Point_2 &o,
                            const FT offset_size) const
    {
        return first_intersection(p, q, o, offset_size, 0.0001);
    }

    void add_point_set(const Points& points) {
        tree_.insert(points.cbegin(), points.cend());
    }

    void load_points(const std::string& filename) {
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

    Tree tree_;
    BBox bbox_;
};
}

