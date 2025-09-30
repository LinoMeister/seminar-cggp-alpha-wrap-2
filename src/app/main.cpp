#include <alpha_wrap_2/alpha_wrap_2.h>
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

  aw2::Oracle oracle;
  std::string filename = "/mnt/storage/repos/HS25/seminar-cg-gp/visual-tools/points_sparse.pts";

  oracle.load_points(filename);

  aw2::alpha_wrap_2 aw(oracle);
  aw.compute_wrap(50, 2);

}
