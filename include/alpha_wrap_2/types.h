//
// Copied public header for types
//

#ifndef AW2_TYPES_H
#define AW2_TYPES_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_data_structure_2.h>

namespace aw2
{

    enum FaceLabel { INSIDE, OUTSIDE };

    typedef double FT;
    typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
    typedef K::Point_2 Point_2;
    typedef std::vector<Point_2> Points;
    typedef K::Segment_2 Segment_2;

    typedef CGAL::Triangulation_vertex_base_2<K> Vb;
    typedef CGAL::Triangulation_face_base_with_info_2<FaceLabel, K, CGAL::Triangulation_face_base_2<K>> Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;

    typedef CGAL::Delaunay_triangulation_2<K, Tds> Delaunay;
    typedef Delaunay::Vertex_handle Vertex_handle;
    typedef Delaunay::Face_handle Face_handle;
}

#endif // AW2_TYPES_H
