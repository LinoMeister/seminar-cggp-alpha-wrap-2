//
// Created by lino on 29.09.25.
//

#ifndef EXPORT_UTILS_H
#define EXPORT_UTILS_H

#include "point_set_oracle_2.h"
#include "types.h"
#include <fstream>
#include <limits>
#include <iomanip>

namespace aw2 {

    using Oracle = point_set_oracle_2;

    void export_svg(const Oracle& oracle, const Delaunay& dt, const std::string& filename,
                    double margin = 50.0, double stroke_width = 2,
                    double vertex_radius = 0.5);
    
}



#endif //EXPORT_UTILS_H
