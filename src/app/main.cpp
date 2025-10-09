#include <alpha_wrap_2/alpha_wrap_2.h>
#include <alpha_wrap_2/export_utils.h>
#include <alpha_wrap_2/types.h>

#include <CGAL/optimal_bounding_box.h>

struct bounding_box {
  aw2::Point_2 min_min;
  aw2::Point_2 min_max;
  aw2::Point_2 max_min;
  aw2::Point_2 max_max;
};

int main(int argc, char *argv[])
{
  // get file path from command line arguments
  // if no argument is given, use a default file path

  aw2::Oracle oracle;
  std::string filename = argc > 1 ? argv[1] : "/mnt/storage/repos/HS25/seminar-cg-gp/visual-tools/points_dense.pts";

  oracle.load_points(filename);

  aw2::FT alpha = argc > 2 ? std::stod(argv[2]) : 10.0;
  aw2::FT offset = argc > 3 ? std::stod(argv[3]) : 2.0;

  aw2::alpha_wrap_2 aw(oracle);

  aw2::AlgorithmConfig config;
  config.alpha = alpha;
  config.offset = offset;
  config.intermediate_steps = 1;
  config.export_step_limit = 10;
  config.max_iterations = 5000;

  aw.compute_wrap(config);

  return 0;
}
