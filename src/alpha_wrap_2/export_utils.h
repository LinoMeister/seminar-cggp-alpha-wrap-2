//
// Created by lino on 29.09.25.
//

#ifndef EXPORT_UTILS_H
#define EXPORT_UTILS_H

#include "types.h"
#include <fstream>
#include <limits>
#include <iomanip>

namespace aw2 {
    void export_svg(const Delaunay& dt, const Oracle& oracle, const std::string& filename,
                    double margin = 50.0, double stroke_width = 2,
                    double vertex_radius = 0.5)
    {
        // First, compute bounding box of finite vertices
        double xmin = oracle.bbox_.x_min;
        double ymin = oracle.bbox_.y_min;
        double xmax = oracle.bbox_.x_max;
        double ymax = oracle.bbox_.y_max;


        if (xmin > xmax || ymin > ymax) {
            // No finite vertices
            return;
        }

        double width = xmax - xmin;
        double height = ymax - ymin;

        // Make an SVG canvas somewhat larger (margin)
        double svg_w = width + 2*margin;
        double svg_h = height + 2*margin;

        std::ofstream os(filename);
        os << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
        os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" )";
        os << "width=\"" << svg_w << "\" height=\"" << svg_h << "\">\n";

        // A helper to map a point to SVG coordinates
        auto to_svg = [&](const Point_2& p) {
            double x = (p.x() - xmin) + margin;
            // Flip Y so that larger y goes downward in SVG coordinate
            double y = (ymax - p.y()) + margin;
            return std::pair<double, double>(x, y);
        };

        // Draw edges of all finite faces
        os << R"(  <g stroke="black" stroke-width=")" << stroke_width
        << "\" fill=\"none\">\n";
        for (auto fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); ++fit) {
            Point_2 pa = fit->vertex(0)->point();
            Point_2 pb = fit->vertex(1)->point();
            Point_2 pc = fit->vertex(2)->point();
            auto sa = to_svg(pa);
            auto sb = to_svg(pb);
            auto sc = to_svg(pc);

            auto inside = (fit->info() == INSIDE);

            if (inside) {
                os << "    <polygon points=\""
                << std::fixed << std::setprecision(3)
                << sa.first << "," << sa.second << " "
                << sb.first << "," << sb.second << " "
                << sc.first << "," << sc.second
                << "\" fill=\"lightblue\" stroke=\"gray\" stroke-width=\"" << stroke_width/2 << "\" />\n";
            }
            else {
                os << "    <polygon points=\""
                << std::fixed << std::setprecision(3)
                << sa.first << "," << sa.second << " "
                << sb.first << "," << sb.second << " "
                << sc.first << "," << sc.second
                << "\" fill=\"none\" stroke=\"gray\" stroke-width=\"" << stroke_width/2 << "\" />\n";
            }

        }
        os << "  </g>\n";

        // Draw vertices as small circles
        os << "  <g stroke=\"red\" stroke-width=\""<< stroke_width <<"\" fill=\"red\">\n";
        for (auto vit = dt.finite_vertices_begin(); vit != dt.finite_vertices_end(); ++vit) {
            const Point_2& p = vit->point();
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius << "\" />\n";
        }
        os << "  </g>\n";

        os << "  <g stroke=\"green\" stroke-width=\""<< stroke_width <<"\" fill=\"green\">\n";
        for (auto vit = oracle.tree_.begin(); vit != oracle.tree_.end(); ++vit) {
            const Point_2& p = *vit;
            std::cout << "Pasting point " << p << std::endl;
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius << "\" />\n";
        }
        os << "  </g>\n";

        os << "</svg>\n";
        os.close();
    }
}



#endif //EXPORT_UTILS_H
