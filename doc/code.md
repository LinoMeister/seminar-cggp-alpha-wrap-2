

# Code

This is just a brief explanation of all the code components. The headers for these components are defined in `include/alpha_wrap_2` with the implementation located in `src/alpha_wrap_2`.

## Core Components

### `point_set_oracle_2`

Allows us to perform checks on the input surface, and implicitly represents the offset surface through an intersection interface.

**Input Geometry**
- For a given query point: find the closest input point. (used for projecting onto the input geometry)
- For a given query triangle: check if the triangle overlaps any input points. (used to check if we can carve a triangle without exposing the input geometry)

**Offset Surface**
- For a given segment `(p,q)` allows us to find the first intersection (starting from `p`) with the offset surface.

Internally a KD-tree is used to organize the input points and efficiently perform the above mentioned operations.

Usage: Create an oracle object and load a point cloud from a file.
```cpp
aw2::Oracle oracle;
oracle.load_points(filename);
```

The file format used is a simple text file with a list of 2D points:
```
283 10
282 11
281 11
280 11
...
```

### `alpha_wrap_2`

Contains the core logic of the alpha wrapping algorithm. The `init()` method applies a configuration and the `run()` method contains the main loop of the algorithm.

Usage: An `alpha_wrap_2` object needs to be initialized with an oracle, then it needs to be set up with a configuration containing all the parameters. After that the algorithm can be executed. See `main.cpp` for more details on how to use the implementation.

```cpp
aw2::alpha_wrap_2 aw(oracle);
aw.init(config);
aw.run();
```


### `traversability`

Contains the logic related to determine if a gate is deemed traversable or not.

3 Different traversability criteria are available:
- `CONSTANT_ALPHA`: Uses a global $\alpha$ which is compared with the minimum Delaunay radius the gate
- `DEVIATION_BASED`:  Adaptively chooses $\alpha$ by measuring deviation from the input to the gate
- `INTERSECTION_BASED`: Adaptively determines traversability by performing intersection tests along the gate

## Utilities

### `export_utils`
Contains utilities to export svg images of the final result and intermediate steps of the algorithm

### `statistics`
Contains utilities for storing statistics when executing the algorithm (like runtime information and algorithm parameters). Also allows us to store the collected statistics into a json file.

### `timer`
A simple timer implementation for timing the total execution time of the algorithm, as well as the runtime of individual parts of the algorithm.

### `types`
Just a collection of type definitions.