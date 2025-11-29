#include <alpha_wrap_2/export_utils.h>
#include <alpha_wrap_2/alpha_wrap_2.h>
#include <utility>
#include <random>
#include <sstream>
#include <cmath>

namespace aw2 {

    alpha_wrap_2_exporter::alpha_wrap_2_exporter(
        const alpha_wrap_2& wrapper, 
        const AlgorithmConfig& config)
        : wrapper_(wrapper), oracle_(wrapper.oracle_), dt_(wrapper.dt_), 
          candidate_gate_(wrapper.candidate_gate_), 
          style_(config.style == "clean" ? StyleConfig::clean_style() :
                 config.style == "outside_filled" ? StyleConfig::outside_filled_style() :
                 StyleConfig::default_style()),
          margin_(style_.margin), 
          stroke_width_(style_.stroke_width), 
          vertex_radius_(style_.vertex_radius),
          inside_rng_(style_.inside_faces.random_seed),
          outside_rng_(style_.outside_faces.random_seed)
    {
        // First, compute bounding box of finite vertices
        xmin_ = wrapper_.dt_bbox_min_.x();
        ymin_ = wrapper_.dt_bbox_min_.y();
        xmax_ = wrapper_.dt_bbox_max_.x();
        ymax_ = wrapper_.dt_bbox_max_.y();
    }

    alpha_wrap_2_exporter::alpha_wrap_2_exporter(
        const alpha_wrap_2& wrapper, 
        const StyleConfig& style)
        : wrapper_(wrapper), oracle_(wrapper.oracle_), dt_(wrapper.dt_), 
          candidate_gate_(wrapper.candidate_gate_), margin_(style.margin), 
          stroke_width_(style.stroke_width), vertex_radius_(style.vertex_radius), style_(style),
          inside_rng_(style.inside_faces.random_seed),
          outside_rng_(style.outside_faces.random_seed)
    {
        // First, compute bounding box of finite vertices
        xmin_ = wrapper_.dt_bbox_min_.x();
        ymin_ = wrapper_.dt_bbox_min_.y();
        xmax_ = wrapper_.dt_bbox_max_.x();
        ymax_ = wrapper_.dt_bbox_max_.y();
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


    void alpha_wrap_2_exporter::export_svg(const std::string& filename, export_context context)
    {

        if (xmin_ > xmax_ || ymin_ > ymax_) {
            return;
        }


        double width = xmax_ - xmin_;
        double height = ymax_ - ymin_;

        // Define viewBox coordinates (automatically fits to container)
        // Y is flipped in SVG, so viewBox uses negative Y coordinates
        double viewbox_x = xmin_ - margin_;
        double viewbox_y = -ymax_ - margin_;
        double viewbox_w = width + 2*margin_;
        double viewbox_h = height + 2*margin_;

        std::ofstream os(export_dir_ / filename);
        os << "<?xml version=\"1.0\" standalone=\"no\"?>\n";
        os << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" )";
        os << "viewBox=\"" << viewbox_x << " " << viewbox_y << " " 
           << viewbox_w << " " << viewbox_h << "\" "
           << "preserveAspectRatio=\"xMidYMid meet\">\n";

        // Write SVG definitions (gradients, patterns, etc.)
        write_svg_defs(os);

        // Draw all finite faces
        os << R"(  <g stroke="black" stroke-width=")" << stroke_width_
        << "\" fill=\"none\">\n";
        
        int face_index = 0;
        for (auto fit = dt_.finite_faces_begin(); fit != dt_.finite_faces_end(); ++fit, ++face_index) {
            auto sa = to_svg(fit->vertex(0)->point());
            auto sb = to_svg(fit->vertex(1)->point());
            auto sc = to_svg(fit->vertex(2)->point());

            bool is_inside = (fit->info() == INSIDE);
            const FaceFillStyle& fill_style = is_inside ? style_.inside_faces : style_.outside_faces;
            
            draw_face(os, sa, sb, sc, fill_style, is_inside, face_index);
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
                auto edge_color = style_.queue_edges.color;
                auto sv1 = to_svg(gate.get_points().first);
                auto sv2 = to_svg(gate.get_points().second);
                draw_line(os, sv1, sv2, edge_color, stroke_width_ * style_.queue_edges.relative_stroke_width);
            }
            os << "  </g>\n";
        }

        // Draw candidate edge

        if (style_.draw_candidate_edge) {
            os << "  <g fill=\"none\">\n";
            auto sv1 = to_svg(candidate_edge_.source());
            auto sv2 = to_svg(candidate_edge_.target());
            draw_line(os, sv1, sv2, style_.candidate_edge.color, 
                     stroke_width_ * style_.candidate_edge.relative_stroke_width);
            os << "  </g>\n";

            if (context == ITERATION_R1) {
                // Draw R1 segment
                os << "  <g fill=\"none\">\n";
                auto r1_sv1 = to_svg(r1_segment_.source());
                auto r1_sv2 = to_svg(r1_segment_.target());
                draw_line(os, r1_sv1, r1_sv2, "#ff00ff", stroke_width_ * 2);
                os << "  </g>\n";

                // Draw Steiner point
                os << "  <g fill=\"none\">\n";
                draw_circle(os, to_svg(steiner_point_), vertex_radius_ * 1.5);
                os << "  </g>\n";
            } else if (context == ITERATION_R2) {
                // Draw R2 segment
                os << "  <g fill=\"none\">\n";
                auto r2_sv1 = to_svg(r2_segment_.source());
                auto r2_sv2 = to_svg(r2_segment_.target());
                draw_line(os, r2_sv1, r2_sv2, "#00ffff", stroke_width_ * 2);
                os << "  </g>\n";

                // Draw Steiner point
                os << "  <g fill=\"none\">\n";
                draw_circle(os, to_svg(steiner_point_), vertex_radius_ * 1.5);
                os << "  </g>\n";
            }
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
            draw_circle(os, sp, style_.input_points.relative_stroke_width);
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
        // Use actual coordinates directly - viewBox handles the coordinate system
        double x = p.x();
        // Flip Y so that larger y goes downward in SVG coordinate
        double y = -p.y();
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

    void alpha_wrap_2_exporter::write_svg_defs(std::ofstream& os) {
        os << "  <defs>\n";
        
        // Note: Gradients for faces are now generated inline with random orientations
        // No need to pre-define them here
        
        // Radial gradient for vertices
        os << "    <radialGradient id=\"vertexGradient\" cx=\"50%\" cy=\"50%\" r=\"50%\" color-interpolation=\"sRGB\">\n";
        os << "      <stop offset=\"0%\" style=\"stop-color:#ffffff;stop-opacity:1\" />\n";
        os << "      <stop offset=\"100%\" style=\"stop-color:#ff4444;stop-opacity:1\" />\n";
        os << "    </radialGradient>\n";
        
        os << "  </defs>\n";
    }
    
    void alpha_wrap_2_exporter::write_gradient_def(std::ofstream& os, const std::string& id,
                                                    const std::string& start_color, 
                                                    const std::string& end_color,
                                                    double angle_degrees) {
        // Convert angle to x1,y1,x2,y2 coordinates
        // Angle of 0° = horizontal (left to right)
        // Angle of 90° = vertical (top to bottom)
        // Angle of 180° = horizontal (right to left)
        // Angle of 270° = vertical (bottom to top)
        double angle_rad = angle_degrees * M_PI / 180.0;
        
        // Calculate gradient vector endpoints
        // We want the gradient to go from one side to the opposite side
        double x1 = 50.0 - 50.0 * std::cos(angle_rad);
        double y1 = 50.0 - 50.0 * std::sin(angle_rad);
        double x2 = 50.0 + 50.0 * std::cos(angle_rad);
        double y2 = 50.0 + 50.0 * std::sin(angle_rad);
        
        os << "    <linearGradient id=\"" << id 
           << "\" x1=\"" << x1 << "%\" y1=\"" << y1 << "%\" "
           << "x2=\"" << x2 << "%\" y2=\"" << y2 << "%\" color-interpolation=\"sRGB\">\n";
        os << "      <stop offset=\"0%\" style=\"stop-color:" << start_color << ";stop-opacity:1\" />\n";
        os << "      <stop offset=\"100%\" style=\"stop-color:" << end_color << ";stop-opacity:1\" />\n";
        os << "    </linearGradient>\n";
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

    // RGBColor utility methods
    std::string RGBColor::to_hex() const {
        std::ostringstream oss;
        oss << "#" << std::hex << std::setfill('0') 
            << std::setw(2) << r 
            << std::setw(2) << g 
            << std::setw(2) << b;
        return oss.str();
    }

    void RGBColor::clamp() {
        r = std::max(0, std::min(255, r));
        g = std::max(0, std::min(255, g));
        b = std::max(0, std::min(255, b));
    }

    RGBColor RGBColor::vary(double variation, std::mt19937& rng) const {
        if (variation <= 0.0) {
            return *this;
        }
        
        std::uniform_real_distribution<double> dist(-variation, variation);
        
        // Apply variation as a percentage of the color value
        auto d = dist(rng);
        int new_r = static_cast<int>(r + d * 255);
        int new_g = static_cast<int>(g + d * 255);
        int new_b = static_cast<int>(b + d * 255);
        
        RGBColor result(new_r, new_g, new_b);
        result.clamp();
        return result;
    }

    // Face drawing helper methods
    void alpha_wrap_2_exporter::draw_face(std::ofstream& os,
                                          const std::pair<double, double>& p1,
                                          const std::pair<double, double>& p2,
                                          const std::pair<double, double>& p3,
                                          const FaceFillStyle& fill_style,
                                          bool is_inside,
                                          int face_index) {
        if (fill_style.mode == FillMode::NONE) {
            draw_polygon(os, p1, p2, p3, "none", style_.delaunay_edges.color, 
                        stroke_width_/2, 1.0);
        } else if (fill_style.mode == FillMode::GRADIENT) {
            // Generate gradient definition inline with random angle
            std::mt19937 rng(fill_style.random_seed + face_index);
            std::uniform_real_distribution<double> angle_dist(0.0, 360.0);
            double random_angle = angle_dist(rng);
            
            std::string gradient_id = get_gradient_id(fill_style, is_inside, face_index);
            
            // Write inline gradient definition
            os << "    <defs>\n";
            write_gradient_def(os, gradient_id, fill_style.gradient_start, fill_style.gradient_end, random_angle);
            os << "    </defs>\n";
            
            // Draw polygon with gradient
            std::string fill_color = "url(#" + gradient_id + ")";
            draw_polygon(os, p1, p2, p3, fill_color, style_.delaunay_edges.color,
                        stroke_width_/2, fill_style.opacity);
        } else {
            std::string fill_color = get_face_fill_color(fill_style, is_inside, face_index);
            draw_polygon(os, p1, p2, p3, fill_color, style_.delaunay_edges.color,
                        stroke_width_/2, fill_style.opacity);
        }
    }

    std::string alpha_wrap_2_exporter::get_face_fill_color(const FaceFillStyle& style, bool is_inside, int face_index) {
        switch (style.mode) {
            case FillMode::NONE:
                return "none";
                
            case FillMode::SOLID:
                return style.base_color;
                
            case FillMode::GRADIENT:
                return "url(#" + get_gradient_id(style, is_inside, face_index) + ")";
                
            case FillMode::VARIED: {
                RGBColor base(style.base_color);
                // Use face_index to seed variation for consistency
                std::mt19937 local_rng(style.random_seed + face_index);
                RGBColor varied = base.vary(style.color_variation, local_rng);
                return varied.to_hex();
            }
            
            default:
                return "none";
        }
    }

    std::string alpha_wrap_2_exporter::get_gradient_id(const FaceFillStyle& /* style */, bool is_inside, int face_index) const {
        return (is_inside ? "insideGradient_" : "outsideGradient_") + std::to_string(face_index);
    }

}
