#include <alpha_wrap_2/export_utils.h>
#include <alpha_wrap_2/alpha_wrap_2.h>

namespace aw2 {

    void export_svg(const alpha_wrap_2& wrapper, const std::string& filename,
                    double margin, double stroke_width,
                    double vertex_radius)
    {
        const auto& oracle_ = wrapper.oracle_;
        const auto& dt_ = wrapper.dt_;
        const auto& candidate_gate_ = wrapper.candidate_gate_;

        // First, compute bounding box of finite vertices
        double xmin = oracle_.bbox_.x_min;
        double ymin = oracle_.bbox_.y_min;
        double xmax = oracle_.bbox_.x_max;
        double ymax = oracle_.bbox_.y_max;


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
        for (auto fit = dt_.finite_faces_begin(); fit != dt_.finite_faces_end(); ++fit) {
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
        for (auto vit = dt_.finite_vertices_begin(); vit != dt_.finite_vertices_end(); ++vit) {
            const Point_2& p = vit->point();
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius << "\" />\n";
        }
        os << "  </g>\n";

        os << "  <g stroke=\"green\" stroke-width=\""<< stroke_width <<"\" fill=\"green\">\n";
        for (auto vit = oracle_.tree_.begin(); vit != oracle_.tree_.end(); ++vit) {
            const Point_2& p = *vit;
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius << "\" />\n";
        }
        os << "  </g>\n";


        // Draw Voronoi diagram (dual of Delaunay triangulation)
        os << "  <g stroke=\"orange\" stroke-width=\"" << stroke_width/2 << "\" fill=\"none\">\n";
        for (auto eit = dt_.finite_edges_begin(); eit != dt_.finite_edges_end(); ++eit) {
            auto face = eit->first;
            int i = eit->second;
            auto neighbor = face->neighbor(i);
            // Only draw each Voronoi edge once
            if (dt_.is_infinite(face) || dt_.is_infinite(neighbor) || face > neighbor) continue;

            CGAL::Object o1 = dt_.dual(eit);
            if (const Segment_2* s = CGAL::object_cast<Segment_2>(&o1)) {

                Point_2 p(s->source().x(), s->source().y());
                Point_2 q(s->target().x(), s->target().y());
                Point_2 o;
                auto intersects = oracle_.first_intersection(p, q, o, 5, 1.0);

                auto sa = to_svg(s->source());
                auto sb = to_svg(s->target());

                if (intersects) {
                    os << "    <line stroke=\"red\" x1=\"" << sa.first << "\" y1=\"" << sa.second
                    << "\" x2=\"" << sb.first << "\" y2=\"" << sb.second << "\" />\n";
                    auto so = to_svg(o);
                    os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
                    << so.first << "\" cy=\"" << so.second
                    << "\" r=\"" << vertex_radius*2 << "\" fill=\"purple\" />\n";
                }
                else {
                    os << "    <line stroke=\"orange\" x1=\"" << sa.first << "\" y1=\"" << sa.second
                    << "\" x2=\"" << sb.first << "\" y2=\"" << sb.second << "\" />\n";
                }



            }
        }
        os << "  </g>\n";

        // draw candidate edge
        os << "  <g stroke=\"green\" stroke-width=\"" << stroke_width/2 << "\" fill=\"none\">\n";
        auto v1 = candidate_gate_.edge.first->vertex(candidate_gate_.edge.first->cw(candidate_gate_.edge.second))->point();
        auto v2 = candidate_gate_.edge.first->vertex(candidate_gate_.edge.first->ccw(candidate_gate_.edge.second))->point();
        auto sv1 = to_svg(v1);
        auto sv2 = to_svg(v2);
        os << "    <line stroke=\"green\" x1=\"" << sv1.first << "\" y1=\"" << sv1.second
        << "\" x2=\"" << sv2.first << "\" y2=\"" << sv2.second << "\" />\n";
        os << "  </g>\n";


        os << "</svg>\n";
        os.close();
    }

}
