# Alpha Wrapping with an Offset in 2D

An implementation of 2D Alpha Wrapping with an offset for point cloud data.

## Dependencies

- **C++17** compatible compiler
- **CMake** 3.16 or higher
- **CGAL** (I only tested it with version 5.6)
- **nlohmann/json** (automatically fetched via CMake FetchContent)

## Building

### Quick Start

```bash
cmake --preset default
cmake --build build
./build/src/app/alpha_wrap2_app --help
```

### Build Configurations

The project includes several CMake presets for different algorithm variants:

- `default`: Standard algorithm with priority queue
- `alternative-trav`: Enables modified alpha traversability computation
- `stack-queue`: Uses stack instead of priority queue


## Usage

### Basic Example

```bash
./build/src/app/alpha_wrap2_app \
  --input path/to/points.pts \
  --output ./data/results \
  --output_use_subdir true \
  --alpha 0.01 \
  --offset 0.01 \
  --traversability CONSTANT_ALPHA
```

### Command Line Options

#### Required Options

- `--input <file>`
  - Path to input point set file (`.pts` format)

- `--output <directory>`
  - Output directory for results (SVG visualizations and statistics)
  - Directory will be created if it doesn't exist

#### Core Algorithm Parameters

>[!IMPORTANT] 
>The alpha and offset parameters are specified relative to a reference length (diagonal length of input bounding box). Hence the actual values used in the algorithm are `alpha*bbox_diagonal_length` and `offset*bbox_diagonal_length`.

- `--alpha <value>`
  - Alpha parameter controlling gate traversability
  - Determines the maximum circumradius of triangles that can be "traversed"
  - Larger values allow larger triangles, resulting in coarser wraps
  - Default: 0.01

- `--offset <value>`
  - Offset distance from input points defining the offset surface
  - Must be positive
  - Default: 0.01

- `--traversability <method>`
  - Traversability computation method
  - Available methods: `CONSTANT_ALPHA`, `DEVIATION_BASED` and `INTERSECTION_BASED`: 
  - Default: `CONSTANT_ALPHA`

- `--max_iterations <count>`
  - Maximum number of algorithm iterations before stopping
  - Default: `50000`

#### Output Options

- `--output_use_subdir <true|false>`
  - If `true`, creates a timestamped subdirectory for outputs
  - Subdirectory name format: Unix timestamp (e.g., `1760343244`)
  - Default: `false`

- `--style <style>`
	- style preset for svg export
  - Available styles:
    - `default`: Shows all faces with different colors for INSIDE/OUTSIDE
    - `clean`: Minimal visualization with cleaner appearance
    - `outside_filled`: Fills OUTSIDE faces for better boundary visibility
  - Default: `default`

- `--intermediate_steps <count>`
  - Number of iterations between intermediate SVG exports
  - Set to 0 to disable intermediate exports
  - Default: `200`

- `--export_step_limit <count>`
  - Maximum number of intermediate SVG exports to create
  - Prevents excessive file creation during long runs
  - Default: `2000`

#### Help

- `--help`
  - Display usage information and exit
  - Example: `./build/src/app/alpha_wrap2_app --help`

### Output Files

Each run produces:
- `final_result.svg`: Final wrap boundary visualization
- `in_progress_iter_N.svg`: Intermediate visualizations (if enabled)
- `statistics.json`: Complete run metadata including configuration, timings, and iteration counts
