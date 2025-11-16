// Public export utilities header
#ifndef AW2_EXPORT_UTILS_H
#define AW2_EXPORT_UTILS_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include <fstream>
#include <limits>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

namespace aw2 {

    using Oracle = point_set_oracle_2;
    
    // Forward declaration
    class alpha_wrap_2;
    struct Gate;

    // Color scheme options
    enum class ColorScheme {
        SIMPLE,
        GRADIENT,
        DATA_MAPPED,
        HEAT_MAP
    };

    struct SimpleStyle {
        std::string color = "white";
        double opacity = 1.0;
        double relative_stroke_width = 1.0; // relative to base stroke width
    };

    // Style configuration
    struct StyleConfig {
        ColorScheme scheme = ColorScheme::SIMPLE;
        bool use_gradients = false;
        bool use_opacity = false;
        double opacity = 1.0;
        std::string gradient_start = "#0e5086";
        std::string gradient_end = "#4ecdc4";
        bool map_to_data = false;
        std::string data_property = "area"; // "area", "circumradius", "quality"

        // Outside face filling options
        bool fill_outside_faces = false;
        bool use_gradients_outside = false;
        double opacity_outside = 0.3;
        std::string gradient_start_outside = "#ff6b6b";
        std::string gradient_end_outside = "#ffd93d";

        double stroke_width = 2.0;
        double vertex_radius = 3.0;
        double input_point_radius = 1.5;
        double margin = 50;

        // element styles
        SimpleStyle input_points = {"black", 0.5};
        SimpleStyle voronoi_diagram = {"orange", 0.5};
        SimpleStyle delaunay_edges = {"gray", 1.0};
        SimpleStyle queue_edges = {"blue", 1.0};
        SimpleStyle candidate_edge = {"green", 1.0};

        bool draw_voronoi_diagram = false;
        bool draw_queue_edges = true;
        bool draw_candidate_edge = true;

        static StyleConfig default_style() {
            StyleConfig style;
            style.voronoi_diagram = {"pink", 0.6};
            style.use_gradients = true;
            style.use_opacity = true;
            style.opacity = 1.0;
            style.scheme = ColorScheme::GRADIENT;
            style.queue_edges = {"#ff8800", 1.0, 2.0};
            style.candidate_edge = {"#225706", 1.0, 2.0};
            style.margin = 15;
            return style;
        }

        static StyleConfig clean_style() {
            StyleConfig style;
            style.use_gradients = true;
            style.use_opacity = true;
            style.opacity = 1.0;
            style.scheme = ColorScheme::GRADIENT;
            style.draw_voronoi_diagram = false;
            style.draw_queue_edges = false;
            style.draw_candidate_edge = false;
            style.delaunay_edges = {"#000000", 1.0};
            style.margin = 15;
            return style;
        }

        static StyleConfig outside_filled_style() {
            StyleConfig style;
            style.use_gradients = true;
            style.use_opacity = true;
            style.opacity = 1.0;
            style.scheme = ColorScheme::GRADIENT;
            
            // Enable outside face filling with different colors
            style.fill_outside_faces = true;
            style.use_gradients_outside = true;
            style.opacity_outside = 0.3;
            style.gradient_start_outside = "#ce2813ff";
            style.gradient_end_outside = "#ffd93d";
            
            style.margin = 15;
            return style;
        }
    };


    class alpha_wrap_2_exporter {
    public:
        alpha_wrap_2_exporter(const alpha_wrap_2& wrapper, const StyleConfig& style = StyleConfig{});

        void export_svg(const std::string& filename);
        void setup_export_dir(const std::string& base_path);
        
        fs::path export_dir_;
        StyleConfig style_;
    
    private:
        void draw_input_points(std::ofstream& os);
        void draw_voronoi_diagram(std::ofstream& os);
        std::pair<double, double> to_svg(const Point_2& p);
        
        // SVG helper methods

        void draw_line(std::ofstream& os, const std::pair<double, double>& p1, 
                      const std::pair<double, double>& p2, const std::string& color,
                      double stroke_width);
        void draw_polygon(std::ofstream& os, const std::pair<double, double>& p1,
                         const std::pair<double, double>& p2, const std::pair<double, double>& p3,
                         const std::string& fill, const std::string& stroke,
                         double stroke_width, double opacity = 1.0);
        void draw_circle(std::ofstream& os, const std::pair<double, double>& center,
                        double radius);
        
        // Enhanced styling methods
        void write_svg_defs(std::ofstream& os, const StyleConfig& style);


        const alpha_wrap_2& wrapper_;
        const Oracle& oracle_;
        const Delaunay& dt_;
        const Gate& candidate_gate_;

        double margin_;
        double stroke_width_;
        double vertex_radius_;

        double xmin_;
        double ymin_;
        double xmax_;
        double ymax_;
    };

        // Add this new class before the alpha_wrap_2_exporter class
    struct RGBColor {
        int r, g, b;
        RGBColor(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
        
        // Constructor from hex string (e.g., "#FF0000")
        RGBColor(const std::string& hex);
        
        std::string to_string() const;
    };

    class ColorMap {
    private:
        RGBColor min_color_;
        RGBColor max_color_;
        double min_value_;
        double max_value_;
        
    public:
        ColorMap(const RGBColor& min_color, const RGBColor& max_color, 
                 double min_value, double max_value);
        
        // Get interpolated color for a value between min_value and max_value
        RGBColor get_color(double value) const;
        
        // Get interpolated color as string
        std::string get_color_string(double value) const;
        
        // Update the value range
        void set_range(double min_value, double max_value);
        
        // Update the colors
        void set_colors(const RGBColor& min_color, const RGBColor& max_color);
    };
}


#endif // AW2_EXPORT_UTILS_H
