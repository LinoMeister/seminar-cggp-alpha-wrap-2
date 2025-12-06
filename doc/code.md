# Code

This is just a brief explanation of all the code components. The headers for these components are defined in `include/alpha_wrap_2` with the implementation located in `src/alpha_wrap_2`.

## Core Components

### `alpha_wrap_2`

Contains the core logic of the alpha wrapping algorithm.

### `point_set_oracle_2`

Allows us to perform checks on the input surface, and implicitly represents the offset surface through an intersection interface.

**Input Geometry**
- For a given query point: find the closest input point. (used for projecting onto the input geometry)
- For a given query triangle: check if the triangle overlaps any input points. (used to check if we can carve a triangle without exposing the input geometry)

**Offset Surface**
- For a given segment `(p,q)` allows us to find the first intersection (starting from `p`) with the offset surface.

Internally a KD-tree is used to organize the input points and efficiently perform the above mentioned operations.

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