//
// Created by lino on 28.09.25.
//

#ifndef TYPES_H
#define TYPES_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>


namespace aw2 {
    typedef double FT;
    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
    typedef K::Point_2 Point_2;
    typedef std::vector<Point_2> Points;
    typedef K::Segment_2 Segment_2;
    typedef CGAL::Delaunay_triangulation_2<K>  Delaunay;
    typedef Delaunay::Vertex_handle           Vertex_handle;
    typedef Delaunay::Face_handle             Face_handle;
}




#endif //TYPES_H
