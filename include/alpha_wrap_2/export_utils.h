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

    class alpha_wrap_2_exporter {
    public:
        alpha_wrap_2_exporter(const alpha_wrap_2& wrapper, 
                              double margin = 50.0, double stroke_width = 2,
                              double vertex_radius = 0.5);

        void export_svg(const std::string& filename);
    
    private:
        void draw_voronoi_diagram(std::ofstream& stream );
        std::pair<double, double> to_svg(const Point_2& p);


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
