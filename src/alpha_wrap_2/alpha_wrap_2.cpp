#include "alpha_wrap_2.h"
#include "types.h"
#include "export_utils.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <iostream>


namespace aw2 {

    void alpha_wrap_2(const Oracle& oracle) {

        std::vector<Point_2> pts_test = {
            {0,0}, {700,0}, {0,500}, {250,800}, {670, 200}, {400, 700}
        };

        auto bbox = oracle.bbox_;
        std::vector<Point_2> pts_bbox = {
            {bbox.x_min,bbox.y_min}, {bbox.x_min,bbox.y_max}, {bbox.x_max,bbox.y_min}, {bbox.x_max,bbox.y_max}
        };

        Delaunay dt;
        dt.insert(pts_bbox.begin(), pts_bbox.end());

        Points pts = {
            {100,100}, {200,200}, {300,300}, {400,400}, {500,500},
            {600,100}, {100,600}, {300,500}, {500,300},
            {150,400}, {400,150}, {250,250}, {350,350}
        };
        dt.insert(pts.begin(), pts.end());

        for (auto fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); ++fit) {
            auto intersects = oracle.do_intersect(fit);
            if (intersects) {
                fit->info() = INSIDE;
            } else {
                fit->info() = OUTSIDE;  
            }
        }

        std::cout << "exporting svg image" << std::endl;
        export_svg(dt, oracle, "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");
    }
}

