// Public export utilities header
#ifndef AW2_EXPORT_UTILS_H
#define AW2_EXPORT_UTILS_H

#include "alpha_wrap_2/types.h"
#include "alpha_wrap_2/point_set_oracle_2.h"
#include <fstream>
#include <limits>
#include <iomanip>

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

    // Style configuration
    struct StyleConfig {
        ColorScheme scheme = ColorScheme::SIMPLE;
        bool use_gradients = false;
        bool use_opacity = false;
        double opacity = 0.7;
        std::string gradient_start = "#ff6b6b";
        std::string gradient_end = "#4ecdc4";
        bool map_to_data = false;
        std::string data_property = "area"; // "area", "circumradius", "quality"
    };

    class alpha_wrap_2_exporter {
    public:
        alpha_wrap_2_exporter(const alpha_wrap_2& wrapper, 
                              double margin = 50.0, double stroke_width = 2,
                              double vertex_radius = 0.5);

        void export_svg(const std::string& filename, const StyleConfig& style = StyleConfig{});
    
    private:
        void draw_voronoi_diagram(std::ofstream& stream, std::string color = "orange");
        std::pair<double, double> to_svg(const Point_2& p);
        
        // Enhanced styling methods
        void write_svg_defs(std::ofstream& os, const StyleConfig& style);
        std::string get_triangle_color(const Delaunay::Face_handle& face, const StyleConfig& style);
        std::string map_value_to_color(double value, double min_val, double max_val);
        double compute_triangle_area(const Delaunay::Face_handle& face);
        std::string hsl_to_string(int h, int s, int l);


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
}


#endif // AW2_EXPORT_UTILS_H
