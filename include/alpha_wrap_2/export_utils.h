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

    void export_svg(const alpha_wrap_2& wrapper, const std::string& filename,
                    double margin = 50.0, double stroke_width = 2,
                    double vertex_radius = 0.5);
    
}


#endif // AW2_EXPORT_UTILS_H
