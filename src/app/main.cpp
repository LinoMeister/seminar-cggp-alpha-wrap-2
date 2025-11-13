#include <alpha_wrap_2/alpha_wrap_2.h>
#include <alpha_wrap_2/export_utils.h>
#include <alpha_wrap_2/types.h>

#include <CGAL/optimal_bounding_box.h>

#include <string>
#include <iostream>
#include <algorithm>

struct bounding_box {
  aw2::Point_2 min_min;
  aw2::Point_2 min_max;
  aw2::Point_2 max_min;
  aw2::Point_2 max_max;
};

// Helper function to find command line argument value
std::string get_cmd_option(char **begin, char **end, const std::string &option)
{
  char **itr = std::find(begin, end, option);
  if (itr != end && ++itr != end)
  {
    return std::string(*itr);
  }
  return "";
}

// Helper function to check if option exists
bool cmd_option_exists(char **begin, char **end, const std::string &option)
{
  return std::find(begin, end, option) != end;
}

void print_usage(const char *program_name)
{
  std::cout << "Usage: " << program_name << " [OPTIONS]\n"
            << "Options:\n"
            << "  --input <file>     Input file path \n"
            << "  --alpha <value>    Alpha value\n"
            << "  --offset <value>   Offset value\n"
            << "  --traversability <method>   Traversability method (CONSTANT_ALPHA, ADAPTIVE_ALPHA, DISTANCE_SAMPLING)\n"
            << "  --help             Show this help message\n";
}

int main(int argc, char *argv[])
{
  // Show help if requested
  if (cmd_option_exists(argv, argv + argc, "--help"))
  {
    print_usage(argv[0]);
    return 0;
  }

  // Parse command line arguments with named parameters
  std::string filename = "/mnt/storage/repos/HS25/seminar-cg-gp/visual-tools/points_dense.pts";

  aw2::AlgorithmConfig config;

  
  std::string input_arg = get_cmd_option(argv, argv + argc, "--input");
  if (!input_arg.empty())
  {
    filename = input_arg;
  }

  std::string output_arg = get_cmd_option(argv, argv + argc, "--output");
  if (!output_arg.empty())
  {
    config.output_directory = output_arg;
  }

  std::string output_use_subdir_arg = get_cmd_option(argv, argv + argc, "--output_use_subdir");
  if (!output_use_subdir_arg.empty())
  {
    if (output_use_subdir_arg == "true") {
      fs::path base_export_path(config.output_directory);
      config.output_directory = base_export_path / std::to_string(std::time(nullptr));
    }
  }

  std::string alpha_arg = get_cmd_option(argv, argv + argc, "--alpha");
  if (!alpha_arg.empty())
  {
    config.alpha = std::stod(alpha_arg);
  }

  std::string offset_arg = get_cmd_option(argv, argv + argc, "--offset");
  if (!offset_arg.empty())
  {
    config.offset = std::stod(offset_arg);
  }

  std::string traversability_arg = get_cmd_option(argv, argv + argc, "--traversability");
  aw2::TraversabilityMethod traversability_method = aw2::CONSTANT_ALPHA;
  if (!traversability_arg.empty()) {
      if (traversability_arg == "CONSTANT_ALPHA") {
          traversability_method = aw2::CONSTANT_ALPHA;
          config.traversability_params = aw2::ConstantAlphaParams{};
      } else if (traversability_arg == "ADAPTIVE_ALPHA") {
          traversability_method = aw2::ADAPTIVE_ALPHA;
          config.traversability_params = aw2::AdaptiveAlphaParams{};
      } else if (traversability_arg == "DISTANCE_SAMPLING") {
          traversability_method = aw2::DISTANCE_SAMPLING;
          config.traversability_params = aw2::DistanceSamplingParams{};
      } else {
          std::cerr << "Unknown traversability method: " << traversability_arg << std::endl;
          return 1;
      }
      config.traversability_method = traversability_method;
  }


  config.intermediate_steps = 200;
  config.export_step_limit = 2000;
  config.max_iterations = 50000;

  std::string intermediate_step_arg = get_cmd_option(argv, argv + argc, "--intermediate_steps");
  if (!intermediate_step_arg.empty())
  {
    config.intermediate_steps = std::stoi(intermediate_step_arg);
  }

  std::string export_step_limit_arg = get_cmd_option(argv, argv + argc, "--export_step_limit");
  if (!export_step_limit_arg.empty())
  {
    config.export_step_limit = std::stoi(export_step_limit_arg);
  }

  std::string max_iter_arg = get_cmd_option(argv, argv + argc, "--max_iterations");
  if (!max_iter_arg.empty())
  {
    config.max_iterations = std::stoi(max_iter_arg);
  }

  aw2::Oracle oracle;
  oracle.load_points(filename);

  aw2::alpha_wrap_2 aw(oracle);


  // Set input filename in statistics
  aw.statistics_.config.input_file = filename;

  aw.init(config);
  aw.run();

  return 0;
}
