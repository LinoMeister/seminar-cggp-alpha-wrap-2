# Alpha Wrapping with Offset in 2D

- **Student**: Lino Meister
- **Supervisor**: Ningfeng Zhou 

This project contains an implementation of **alpha wrapping with an offset** for a 2D scenario, focused on 2D point clouds as input. This implementation was further used to conduct experiments with different traversability criteria.

## Documentation

A more detailed documentation is available in `/doc`.
- [Project Overview](doc/project.md): Provides an overview of the project.
- [Experiments](doc/experiments.md): Documents the experiments that were conducted as part of the project.
- [Code](doc/code.md): Describes the involved code components.
- [Supplementary Material](doc/supplementary.md): Overview of supplementary material.


## Building

### Dependencies

- **C++17** compatible compiler
- **CMake** 3.16 or higher
- [**CGAL**](https://www.cgal.org/download.html) (Tested with version 5.6)
- **nlohmann/json** (automatically fetched via CMake FetchContent)

### Quick Start

```bash
cmake --preset default
cmake --build build
./build/src/app/alpha_wrap2_app --help
```

### Build Configurations

The project includes several CMake presets for different algorithm variants. 

#### `default`: Standard algorithm with priority queue

This is the default configuration of the algorithm.

#### `alternative-trav`: Enables modified alpha traversability computation

This configuration was used for a small experiment, using a slightly modified traversability criterion. See the “alternative traversability” section in the [report](doc/experiments.md). Please note that this is different from the adaptive traversability methods. The adaptive methods can be used with the default configuration.


#### `stack-queue`: Uses stack instead of priority queue

The paper describes using a priority queue (sorted by each gate’s minimum Delaunay ball radius). The [CGAL implementation](https://github.com/CGAL/cgal/blob/cb6407e04270becf748a363a2062416f9e5e8513/Alpha_wrap_3/include/CGAL/Alpha_wrap_3/internal/Alpha_wrap_3.h#L147) also offers a stack, which can be faster in practice. I used the priority queue for all experiments but included a stack option.


## Usage

Some example inputs are available in the [supplementary material](doc/supplementary.md). You can place these examples in `data/input`.

### Basic Example

```bash
./build/src/app/alpha_wrap2_app \
  --input data/input/example1_dense.pts \
  --output data/results \
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
>Alpha and offset are specified relative to a reference length (the diagonal of the input bounding box). The algorithm uses `alpha * bbox_diagonal_length` and `offset * bbox_diagonal_length`.

- `--alpha <value>`
  - Alpha parameter controlling gate traversability
  - Small values result in a more exact representation of the offset surface
  - Default: `0.01`

- `--offset <value>`
  - Offset distance from input points defining the offset surface
  - Must be positive
  - Default: `0.01`

- `--traversability <method>`
  - Traversability computation method
  - Available methods: `CONSTANT_ALPHA`, `DEVIATION_BASED`, `INTERSECTION_BASED`
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
  - Style preset for SVG export
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

## Disclaimer

>[!NOTE]
>AI-tools were heavily involved in writing code for utilities (SVG exporting, JSON serialization and timer). Furthermore, AI-tools assisted with project configuration, debugging, review and improvement of the project documentation.
