# Code

This briefly explains the code components. The headers for these components are defined in `include/alpha_wrap_2` with the implementation located in `src/alpha_wrap_2`.

## Core Components

### `point_set_oracle_2`

Allows checks on the input surface and implicitly represents the offset surface through an intersection interface.

**Input Geometry**
- For a given query point: find the closest input point (used to project onto the input geometry).
- For a given query triangle: check if the triangle contains or intersects any input points (used to check if we can carve a triangle without exposing the input geometry).

**Offset Surface**
- For a given segment `(p, q)`, finds the first intersection (starting from `p`) with the offset surface.

Internally a k-d tree is used to organize the input points and efficiently perform the above operations.

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

Contains the core logic of the alpha wrapping algorithm. The `init()` method applies a configuration, and the `run()` method contains the main loop of the algorithm.

Usage: An `alpha_wrap_2` object needs to be initialized with an oracle, then it needs to be set up with a configuration containing all the parameters. After that the algorithm can be executed. See `main.cpp` for more details on how to use the implementation.

```cpp
aw2::alpha_wrap_2 aw(oracle);
aw.init(config);
aw.run();
```

After running the algorithm, the extracted surface is stored as a list of line segments in the variable `aw.wrap_edges_`.

### `traversability`

Contains the logic for determining whether a gate is deemed traversable.

Three traversability criteria are available:
- `CONSTANT_ALPHA`: Uses a global $\alpha$, compared with the gate’s minimum Delaunay radius.
- `DEVIATION_BASED`: Adaptively chooses $\alpha$ by measuring deviation from the input to the gate.
- `INTERSECTION_BASED`: Adaptively determines traversability by performing intersection tests along the gate.

## Utilities

### `export_utils`
Contains utilities to export SVG images of the final result and intermediate steps of the algorithm.

### `statistics`
Contains utilities for storing statistics when executing the algorithm (like runtime and parameters). Also allows storing the collected statistics into a JSON file.

### `timer`
A simple timer for timing the total execution of the algorithm, as well as the runtime of individual parts.

### `types`
Just a collection of type definitions.