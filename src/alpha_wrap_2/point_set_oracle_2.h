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

    bool empty() const;
    bool do_call() const;
    void clear();

    bool do_intersect(const Face_handle &t) const;

    FT squared_distance(const Point_2 &p) const;

    Point_2 closest_point(const Point_2 &p) const;

    bool first_intersection(const Point_2 &p, 
                            const Point_2 &q,
                            Point_2 &o,
                            const FT offset_size,
                            const FT intersection_precision) const;
    

    bool first_intersection(const Point_2 &p, const Point_2 &q,
                            Point_2 &o,
                            const FT offset_size) const;

    void add_point_set(const Points& points);

    void load_points(const std::string& filename);

    Tree tree_;
    BBox bbox_;
};
}

