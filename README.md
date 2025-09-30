# Alpha Wrapping with an Offset in 2D


## TODO

### Offset Surface

- Correct implementation of intersection with offset surface
    - Updated with manual computation of intersection with circle around closest input point

- More efficient computation of intersection with offset surface
    - Currently use brute-force appraoch

- Currently use kd-tree to organize points, maybe not the best choice.


### Core Algorithm

- Correct computation of circum center for infinite faces
    - Updated with a new procedure, still needs to be checked if this is reliable

- Priority queue is currently just a stack
    - Look at the one in CGAL, can probably work similarly

- Correct implementation of alpha traversability check
    - Currently we just use edge length for minimum Delaunay ball radius



### Surface Extraction
- Extract the surface that separates inside from outside