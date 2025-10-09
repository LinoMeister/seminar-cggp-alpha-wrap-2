# Alpha Wrapping with an Offset in 2D

## Commands

```
./alpha_wrap2_app "/mnt/storage/repos/HS25/seminar-cg-gp/visual-tools/points_dense.pts" 10 2
```

## TODO

### Offset Surface

- Correct implementation of intersection with offset surface
    - Updated with manual computation of intersection with circle around closest input point
    - might still note be accurate in some cases!

- More efficient computation of intersection with offset surface
    - Currently use brute-force appraoch
    - Improved by reducing search to a bbox

- Currently use kd-tree to organize points, maybe not the best choice?


### Core Algorithm

- Correct computation of circum center for infinite faces
    - Updated with a new procedure, still needs to be checked if this is reliable

- Priority queue is currently just a stack
    - Look at the one in CGAL, can probably work similarly
    - Added option to use priority queue
    - still need to compare them

- Efficient queue insertion and removal
    - Currently clear queue after each iteration and check every edge for insertion

- Correct implementation of alpha traversability check
    - Currently we just use edge length for minimum Delaunay ball radius
    - Improved with case distinction
        - Still need to verify this, why would it be sufficient for the circumradius of the outer cell to be $\geq \alpha$?


### Surface Extraction
- Extract the surface that separates inside from outside