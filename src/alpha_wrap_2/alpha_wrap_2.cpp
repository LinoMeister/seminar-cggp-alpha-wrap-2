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
        std::vector<Point_2> pts = {
            {bbox.x_min,bbox.y_min}, {bbox.x_min,bbox.y_max}, {bbox.x_max,bbox.y_min}, {bbox.x_max,bbox.y_max}
        };

        Delaunay dt;
        dt.insert(pts.begin(), pts.end());

        std::cout << "exporting svg image" << std::endl;
        export_svg(dt, oracle, "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results/triangulation.svg");
    }
}

