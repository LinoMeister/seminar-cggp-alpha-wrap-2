// Public export utilities header
#ifndef AW2_EXPORT_UTILS_H
#define AW2_EXPORT_UTILS_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include <fstream>
#include <limits>
#include <iomanip>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;

namespace aw2 {

    using Oracle = point_set_oracle_2;
    
    // Forward declaration
    class alpha_wrap_2;
    struct Gate;

    // Forward declarations
    struct RGBColor;
    
    // Fill mode for face rendering
    enum class FillMode {
        NONE,           // No fill
        SOLID,          // Single solid color
        GRADIENT,       // Linear gradient between two colors
        VARIED          // Base color with random variation
    };

    struct SimpleStyle {
        std::string color = "white";
        double opacity = 1.0;
        double relative_stroke_width = 1.0; // relative to base stroke width
    };

    // Configuration for face fill styling
    struct FaceFillStyle {
        FillMode mode = FillMode::NONE;
        double opacity = 1.0;
        
        // For SOLID and VARIED modes
        std::string base_color = "#0e5086";
        
        // For GRADIENT mode
        std::string gradient_start = "#0e5086";
        std::string gradient_end = "#4ecdc4";
        
        // For VARIED mode
        double color_variation = 0.15;  // 0.0 to 1.0, amount of random variation
        unsigned int random_seed = 42;  // For reproducible variation
        
        // Constructor helpers
        static FaceFillStyle none() {
            return FaceFillStyle{FillMode::NONE, 1.0, "", "", "", 0.0, 0};
        }
        
        static FaceFillStyle solid(const std::string& color, double opacity = 1.0) {
            return FaceFillStyle{FillMode::SOLID, opacity, color, "", "", 0.0, 0};
        }
        
        static FaceFillStyle gradient(const std::string& start, const std::string& end, double opacity = 1.0) {
            return FaceFillStyle{FillMode::GRADIENT, opacity, "", start, end, 0.0, 0};
        }
        
        static FaceFillStyle varied(const std::string& base, double variation = 0.15, double opacity = 1.0, unsigned int seed = 42) {
            return FaceFillStyle{FillMode::VARIED, opacity, base, "", "", variation, seed};
        }
    };

    // Style configuration
    struct StyleConfig {
        // Face fill styles
        FaceFillStyle inside_faces = FaceFillStyle::gradient("#0e5086", "#4ecdc4", 1.0);
        FaceFillStyle outside_faces = FaceFillStyle::none();

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
        bool draw_candidate_cc = false;

        static StyleConfig default_style() {
            StyleConfig style;
            style.inside_faces = FaceFillStyle::gradient("#0e80dd", "#18aac4", 1.0);
            style.outside_faces = FaceFillStyle::none();
            style.voronoi_diagram = {"pink", 0.6};
            style.queue_edges = {"#20a83d", 1.0, 2.0};
            style.candidate_edge = {"#ff9900", 1.0, 2.0};
            style.input_points = {"black", 1.0, 2.5};
            style.margin = 15;
            style.draw_candidate_cc = false;
            return style;
        }

        static StyleConfig clean_style() {
            StyleConfig style = default_style();
            style.draw_voronoi_diagram = false;
            style.draw_queue_edges = false;
            style.draw_candidate_edge = false;

            return style;
        }

        static StyleConfig outside_filled_style() {
            StyleConfig style = default_style();
            style.outside_faces = FaceFillStyle::gradient("#ff6b6b", "#ffd93d", 0.3);
            return style;
        }
    };


    // RGB Color utility class
    struct RGBColor {
        int r, g, b;
        RGBColor(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
        
        // Constructor from hex string (e.g., "#FF0000")
        RGBColor(const std::string& hex);
        
        std::string to_string() const;
        std::string to_hex() const;
        
        // Generate a varied color based on this color
        RGBColor vary(double variation, std::mt19937& rng) const;
        
        // Clamp RGB values to valid range
        void clamp();
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
        
        // Face drawing with style support
        void draw_face(std::ofstream& os, 
                      const std::pair<double, double>& p1,
                      const std::pair<double, double>& p2, 
                      const std::pair<double, double>& p3,
                      const FaceFillStyle& fill_style,
                      bool is_inside,
                      int face_index);
        
        std::string get_face_fill_color(const FaceFillStyle& style, bool is_inside, int face_index);
        std::string get_gradient_id(const FaceFillStyle& style, bool is_inside, int face_index) const;
        
        // SVG definitions
        void write_svg_defs(std::ofstream& os);
        void write_gradient_def(std::ofstream& os, const std::string& id, 
                               const std::string& start_color, const std::string& end_color,
                               double angle_degrees = 135.0);


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
        
        // Random number generators for varied colors (mutable to allow use in const-like contexts)
        mutable std::mt19937 inside_rng_;
        mutable std::mt19937 outside_rng_;
    };
}


#endif // AW2_EXPORT_UTILS_H
