#include <alpha_wrap_2/export_utils.h>
#include <alpha_wrap_2/alpha_wrap_2.h>
#include <utility>

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

    void alpha_wrap_2_exporter::setup_export_dir(const std::string& base_path) {

        fs::path base_export_path(base_path);
        export_dir_ = base_export_path;

        if (!fs::exists(base_export_path)) {
            try {
                fs::create_directories(base_export_path);
            } catch (const fs::filesystem_error& e) {
                throw std::runtime_error("Failed to create directory: " + std::string(e.what()));
            }
        }
    }


    void alpha_wrap_2_exporter::export_svg(const std::string& filename)
    {

        if (xmin_ > xmax_ || ymin_ > ymax_) {
            return;
        }

        double width = xmax_ - xmin_;
        double height = ymax_ - ymin_;

        // Make an SVG canvas somewhat larger (margin)
        double svg_w = width + 2*margin_;
        double svg_h = height + 2*margin_;

        std::ofstream os(export_dir_ / filename);
        os << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
        os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" )";
        os << "width=\"" << svg_w << "\" height=\"" << svg_h << "\">\n";

        // Write SVG definitions (gradients, patterns, etc.)
        write_svg_defs(os, style_);


        // Draw edges of all finite faces
        os << R"(  <g stroke="black" stroke-width=")" << stroke_width_
        << "\" fill=\"none\">\n";
        for (auto fit = dt_.finite_faces_begin(); fit != dt_.finite_faces_end(); ++fit) {
            auto sa = to_svg(fit->vertex(0)->point());
            auto sb = to_svg(fit->vertex(1)->point());
            auto sc = to_svg(fit->vertex(2)->point());

            auto inside = (fit->info() == INSIDE);
            double opacity = style_.use_opacity ? style_.opacity : 1.0;

            if (inside) {
                std::string fill_color = style_.use_gradients ? "url(#triangleGradient)" : style_.gradient_start;
                draw_polygon(os, sa, sb, sc, fill_color, style_.delaunay_edges.color, stroke_width_/2, opacity);
            }
            else {
                draw_polygon(os, sa, sb, sc, "none", style_.delaunay_edges.color, stroke_width_/2, 1.0);
            }
        }
        os << "  </g>\n";

        // Draw vertices 
        os << "  <g stroke=\"red\" stroke-width=\""<< stroke_width_ <<"\" fill=\"red\">\n";
        for (auto vit = dt_.finite_vertices_begin(); vit != dt_.finite_vertices_end(); ++vit) {
            auto sp = to_svg(vit->point());
            draw_circle(os, sp, vertex_radius_);
        }
        os << "  </g>\n";

        if (style_.draw_voronoi_diagram) {
            draw_voronoi_diagram(os);
        }

        // Draw input points
        draw_input_points(os);

        // Draw queue edges with priority-based coloring
        if (style_.draw_queue_edges) {
            ColorMap priority_colormap(RGBColor("#08fa00"), RGBColor("#ff1100"), 0, 500);
            Queue temp_queue = wrapper_.queue_;
            os << "  <g fill=\"none\">\n";
            while (!temp_queue.empty()) {
                auto gate = temp_queue.top();
                temp_queue.pop();
                auto edge_color = priority_colormap.get_color(gate.priority).to_string();
                auto sv1 = to_svg(gate.get_points().first);
                auto sv2 = to_svg(gate.get_points().second);
                draw_line(os, sv1, sv2, edge_color, stroke_width_ * style_.queue_edges.relative_stroke_width);
            }
            os << "  </g>\n";
        }

        // Draw candidate edge
        if (candidate_gate_.edge.first != Delaunay::Face_handle()) {
            os << "  <g fill=\"none\">\n";
            auto sv1 = to_svg(candidate_gate_.get_points().first);
            auto sv2 = to_svg(candidate_gate_.get_points().second);
            draw_line(os, sv1, sv2, style_.candidate_edge.color, 
                     stroke_width_ * style_.candidate_edge.relative_stroke_width);
            os << "  </g>\n";
        }

        // Draw extracted wrap edges with adaptive alpha coloring

        os << "  <g fill=\"none\">\n";
        auto wrap_edge_color = RGBColor("#e800fd").to_string();
        for (const auto& seg : wrapper_.wrap_edges_) {
            auto sa = to_svg(seg.source());
            auto sb = to_svg(seg.target());
            draw_line(os, sa, sb, wrap_edge_color, stroke_width_);
        }
        os << "  </g>\n";


        os << "</svg>\n";
        os.close();
    }

    void alpha_wrap_2_exporter::draw_input_points(std::ofstream& os) {
        os << "  <g fill=\"" << style_.input_points.color 
           << "\" opacity=\"" << style_.input_points.opacity << "\">\n";
        for (auto vit = oracle_.tree_.begin(); vit != oracle_.tree_.end(); ++vit) {
            auto sp = to_svg(*vit);
            draw_circle(os, sp, style_.input_point_radius);
        }
        os << "  </g>\n";
    }

    void alpha_wrap_2_exporter::draw_voronoi_diagram(std::ofstream& os) {
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
                auto sa = to_svg(s->source());
                auto sb = to_svg(s->target());
                draw_line(os, sa, sb, style_.voronoi_diagram.color, stroke_width_/2);
            }
        }
        os << "  </g>\n";
    }

    std::pair<double, double> alpha_wrap_2_exporter::to_svg(const Point_2& p) {
        double x = (p.x() - xmin_) + margin_;
        // Flip Y so that larger y goes downward in SVG coordinate
        double y = (ymax_ - p.y()) + margin_;
        return std::pair<double, double>(x, y);
    }

    void alpha_wrap_2_exporter::draw_line(std::ofstream& os, const std::pair<double, double>& p1, 
                                         const std::pair<double, double>& p2, const std::string& color,
                                         double stroke_width) {
        os << "    <line x1=\"" << p1.first << "\" y1=\"" << p1.second
           << "\" x2=\"" << p2.first << "\" y2=\"" << p2.second 
           << "\" stroke=\"" << color << "\" stroke-width=\"" << stroke_width << "\" />\n";
    }

    void alpha_wrap_2_exporter::draw_polygon(std::ofstream& os, const std::pair<double, double>& p1,
                                            const std::pair<double, double>& p2, const std::pair<double, double>& p3,
                                            const std::string& fill, const std::string& stroke,
                                            double stroke_width, double opacity) {
        os << "    <polygon points=\""
           << std::fixed << std::setprecision(3)
           << p1.first << "," << p1.second << " "
           << p2.first << "," << p2.second << " "
           << p3.first << "," << p3.second
           << "\" fill=\"" << fill << "\" fill-opacity=\"" << opacity
           << "\" stroke=\"" << stroke << "\" stroke-width=\"" << stroke_width << "\" />\n";
    }

    void alpha_wrap_2_exporter::draw_circle(std::ofstream& os, const std::pair<double, double>& center,
                                           double radius) {
        os << "    <circle cx=\"" << std::fixed << std::setprecision(3)
           << center.first << "\" cy=\"" << center.second
           << "\" r=\"" << radius << "\" />\n";
    }

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

    RGBColor::RGBColor(const std::string& hex) {
        if (hex.length() != 7 || hex[0] != '#') {
            throw std::invalid_argument("Invalid hex color format. Expected #RRGGBB");
        }
        
        try {
            r = std::stoi(hex.substr(1, 2), nullptr, 16);
            g = std::stoi(hex.substr(3, 2), nullptr, 16);
            b = std::stoi(hex.substr(5, 2), nullptr, 16);
        } catch (const std::exception&) {
            throw std::invalid_argument("Invalid hex color format");
        }
        
        // Clamp values to valid range
        r = std::max(0, std::min(255, r));
        g = std::max(0, std::min(255, g));
        b = std::max(0, std::min(255, b));
    }

    std::string RGBColor::to_string() const {
        std::ostringstream oss;
        oss << "rgb(" << r << "," << g << "," << b << ")";
        return oss.str();
    }

    // ColorMap implementation
    ColorMap::ColorMap(const RGBColor& min_color, const RGBColor& max_color, 
                       double min_value, double max_value)
        : min_color_(min_color), max_color_(max_color), 
          min_value_(min_value), max_value_(max_value) {
        if (max_value <= min_value) {
            throw std::invalid_argument("max_value must be greater than min_value");
        }
    }

    RGBColor ColorMap::get_color(double value) const {

        // Clamp value to valid range
        value = std::max(min_value_, std::min(max_value_, value));
        
        // Normalize value to [0, 1]
        double t = (value - min_value_) / (max_value_ - min_value_);
        
        // Linear interpolation between colors
        int r = static_cast<int>(min_color_.r + t * (max_color_.r - min_color_.r));
        int g = static_cast<int>(min_color_.g + t * (max_color_.g - min_color_.g));
        int b = static_cast<int>(min_color_.b + t * (max_color_.b - min_color_.b));
        
        return RGBColor(r, g, b);
    }

    std::string ColorMap::get_color_string(double value) const {
        return get_color(value).to_string();
    }

    void ColorMap::set_range(double min_value, double max_value) {
        if (max_value <= min_value) {
            throw std::invalid_argument("max_value must be greater than min_value");
        }
        min_value_ = min_value;
        max_value_ = max_value;
    }

    void ColorMap::set_colors(const RGBColor& min_color, const RGBColor& max_color) {
        min_color_ = min_color;
        max_color_ = max_color;
    }

}
