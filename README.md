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
- `modified-algo`: Enables modified alpha traversability computation
- `stack-queue`: Uses stack instead of priority queue
- `all-modifications`: Enables all modifications


## Usage

```bash
./build/src/app/alpha_wrap2_app \
  --input path/to/points.pts \
  --output ./data/results \
  --output_use_subdir true \
  --alpha 10 \
  --offset 3 \
  --traversability CONSTANT_ALPHA
```

### Command Line Options

- `--input`: Path to input point set file (.pts format)
- `--output`: Output directory for results
- `--output_use_subdir`: Create timestamped subdirectory for outputs
- `--alpha`: Alpha parameter controlling traversability
- `--offset`: Offset distance from input points
- `--traversability`: Traversability method (`CONSTANT_ALPHA`, `ADAPTIVE_ALPHA`, or `DISTANCE_SAMPLING`)
