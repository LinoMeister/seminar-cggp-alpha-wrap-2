#include <alpha_wrap_2/export_utils.h>
#include <alpha_wrap_2/alpha_wrap_2.h>

namespace aw2 {

    alpha_wrap_2_exporter::alpha_wrap_2_exporter(
        const alpha_wrap_2& wrapper, 
        const StyleConfig& style)
        : wrapper_(wrapper), oracle_(wrapper.oracle_), dt_(wrapper.dt_), 
          candidate_gate_(wrapper.candidate_gate_), margin_(style.margin), 
          stroke_width_(style.stroke_width), vertex_radius_(style.vertex_radius), style_(style)
    {
        // First, compute bounding box of finite vertices
        xmin_ = oracle_.bbox_.x_min;
        ymin_ = oracle_.bbox_.y_min;
        xmax_ = oracle_.bbox_.x_max;
        ymax_ = oracle_.bbox_.y_max;
    }

    void alpha_wrap_2_exporter::export_svg(const std::string& filename)
    {
        bool draw_voronoi = true;


        if (xmin_ > xmax_ || ymin_ > ymax_) {
            // No finite vertices
            return;
        }

        double width = xmax_ - xmin_;
        double height = ymax_ - ymin_;

        // Make an SVG canvas somewhat larger (margin)
        double svg_w = width + 2*margin_;
        double svg_h = height + 2*margin_;

        std::ofstream os(filename);
        os << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
        os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" )";
        os << "width=\"" << svg_w << "\" height=\"" << svg_h << "\">\n";

        // Write SVG definitions (gradients, patterns, etc.)
        write_svg_defs(os, style_);


        // Draw edges of all finite faces with enhanced styling
        os << R"(  <g stroke="black" stroke-width=")" << stroke_width_
        << "\" fill=\"none\">\n";
        for (auto fit = dt_.finite_faces_begin(); fit != dt_.finite_faces_end(); ++fit) {
            Point_2 pa = fit->vertex(0)->point();
            Point_2 pb = fit->vertex(1)->point();
            Point_2 pc = fit->vertex(2)->point();
            auto sa = to_svg(pa);
            auto sb = to_svg(pb);
            auto sc = to_svg(pc);

            auto inside = (fit->info() == INSIDE);

            std::string fill_color = get_triangle_color(fit, style_);
            double opacity = style_.use_opacity ? style_.opacity : 1.0;

            if (inside) {
                os << "    <polygon points=\""
                << std::fixed << std::setprecision(3)
                << sa.first << "," << sa.second << " "
                << sb.first << "," << sb.second << " "
                << sc.first << "," << sc.second
                << "\" fill=\"" << fill_color << "\" fill-opacity=\"" << opacity
                << "\" stroke=\"" << style_.delaunay_edges.color << "\" stroke-width=\"" << stroke_width_/2 << "\" />\n";
            }
            else {
                os << "    <polygon points=\""
                << std::fixed << std::setprecision(3)
                << sa.first << "," << sa.second << " "
                << sb.first << "," << sb.second << " "
                << sc.first << "," << sc.second
                << "\" fill=\"none\" stroke=\"" << style_.delaunay_edges.color << "\" stroke-width=\"" << stroke_width_/2 << "\" />\n";
            }

        }
        os << "  </g>\n";

        // Draw vertices as small circles
        os << "  <g stroke=\"red\" stroke-width=\""<< stroke_width_ <<"\" fill=\"red\">\n";
        for (auto vit = dt_.finite_vertices_begin(); vit != dt_.finite_vertices_end(); ++vit) {
            const Point_2& p = vit->point();
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << vertex_radius_ << "\" />\n";
        }
        os << "  </g>\n";

        if (draw_voronoi) {
            draw_voronoi_diagram(os);
        }

        // Draw input points
        draw_input_points(os);


        // draw candidate edge
        os << "  <g stroke=\"green\" stroke-width=\"" << stroke_width_/2 << "\" fill=\"none\">\n";
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

    void alpha_wrap_2_exporter::draw_input_points(std::ofstream& os) {
        os << "  <g" << " fill=\"" << style_.input_points.color << "\" opacity=\"" << style_.input_points.opacity << "\">\n";
        for (auto vit = oracle_.tree_.begin(); vit != oracle_.tree_.end(); ++vit) {
            const Point_2& p = *vit;
            auto sp = to_svg(p);
            os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
            << sp.first << "\" cy=\"" << sp.second
            << "\" r=\"" << style_.input_point_radius << "\" />\n";
        }
        os << "  </g>\n";
    }

    void alpha_wrap_2_exporter::draw_voronoi_diagram(std::ofstream& os) {
        // Draw Voronoi diagram (dual of Delaunay triangulation)
    

        os << "  <g stroke=\"" << style_.voronoi_diagram.color << "\""
        << " opacity=\"" << style_.voronoi_diagram.opacity << "\""
        << " stroke-width=\"" << stroke_width_/2 << "\" fill=\"none\">\n";
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

                auto sa = to_svg(s->source());
                auto sb = to_svg(s->target());

                os << "    <line x1=\"" << sa.first << "\" y1=\"" << sa.second
                << "\" x2=\"" << sb.first << "\" y2=\"" << sb.second << "\" />\n";
            }
        }
        os << "  </g>\n";
    }

    std::pair<double, double> alpha_wrap_2_exporter::to_svg(const Point_2& p) {
        double x = (p.x() - xmin_) + margin_;
        // Flip Y so that larger y goes downward in SVG coordinate
        double y = (ymax_ - p.y()) + margin_;
        return std::pair<double, double>(x, y);
    };

    void alpha_wrap_2_exporter::write_svg_defs(std::ofstream& os, const StyleConfig& style) {
        os << "  <defs>\n";
        
        if (style.use_gradients) {
            // Linear gradient for triangles
            os << "    <linearGradient id=\"triangleGradient\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">\n";
            os << "      <stop offset=\"0%\" style=\"stop-color:" << style.gradient_start << ";stop-opacity:1\" />\n";
            os << "      <stop offset=\"100%\" style=\"stop-color:" << style.gradient_end << ";stop-opacity:1\" />\n";
            os << "    </linearGradient>\n";
            
            // Radial gradient for vertices
            os << "    <radialGradient id=\"vertexGradient\" cx=\"50%\" cy=\"50%\" r=\"50%\">\n";
            os << "      <stop offset=\"0%\" style=\"stop-color:#ffffff;stop-opacity:1\" />\n";
            os << "      <stop offset=\"100%\" style=\"stop-color:#ff4444;stop-opacity:1\" />\n";
            os << "    </radialGradient>\n";
        }
        
        // Pattern for special elements
        os << "    <pattern id=\"stripes\" patternUnits=\"userSpaceOnUse\" width=\"4\" height=\"4\">\n";
        os << "      <rect width=\"2\" height=\"4\" fill=\"#cccccc\"/>\n";
        os << "      <rect x=\"2\" width=\"2\" height=\"4\" fill=\"#ffffff\"/>\n";
        os << "    </pattern>\n";
        
        os << "  </defs>\n";
    }

    std::string alpha_wrap_2_exporter::get_triangle_color(const Delaunay::Face_handle& face, const StyleConfig& style) {
        switch (style.scheme) {
            case ColorScheme::GRADIENT:
                return style.use_gradients ? "url(#triangleGradient)" : style.gradient_start;
                
            case ColorScheme::DATA_MAPPED:
                if (style.map_to_data) {
                    double area = compute_triangle_area(face);
                    // You would need to compute min/max areas across all triangles
                    return map_value_to_color(area, 0.0, 100.0); // placeholder values
                }
                return "lightblue";
                
            case ColorScheme::HEAT_MAP:
                // Map based on some property like distance from center
                return hsl_to_string(240, 70, 50); // Blue as default
                
            default: // SIMPLE
                return "lightblue";
        }
    }

    std::string alpha_wrap_2_exporter::map_value_to_color(double value, double min_val, double max_val) {
        if (max_val <= min_val) return "lightblue";
        
        double normalized = std::max(0.0, std::min(1.0, (value - min_val) / (max_val - min_val)));
        
        // Map to hue: blue (240) to red (0)
        int hue = static_cast<int>(240 * (1 - normalized));
        return hsl_to_string(hue, 70, 50);
    }

    double alpha_wrap_2_exporter::compute_triangle_area(const Delaunay::Face_handle& face) {
        Point_2 p1 = face->vertex(0)->point();
        Point_2 p2 = face->vertex(1)->point();
        Point_2 p3 = face->vertex(2)->point();
        
        // Using cross product for area calculation
        double area = std::abs((p2.x() - p1.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p2.y() - p1.y())) / 2.0;
        return area;
    }

    std::string alpha_wrap_2_exporter::hsl_to_string(int h, int s, int l) {
        return "hsl(" + std::to_string(h) + "," + std::to_string(s) + "%," + std::to_string(l) + "%)";
    }

}
