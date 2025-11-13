# Alpha Wrapping with an Offset in 2D

## Commands

```
./alpha_wrap2_app --input "/mnt/storage/repos/HS25/seminar-cg-gp/visual-tools/export/example6_dense.pts" --output "/mnt/storage/repos/HS25/seminar-cg-gp/alpha-wrap-2/data/results" --output_use_subdir true  --alpha 10 --offset 3 --traversability CONSTANT_ALPHA
```

## TODO


### Surface Extraction
- Extract the surface that separates inside from outside


### Offset Surface

- small offsets lead to points that are detected as intersecting with the triangel -> infinite loop
    - maybe attempt to improve robustness

- Correct implementation of intersection with offset surface
    - Updated with manual computation of intersection with circle around closest input point
    - might still note be accurate in some cases!
    - should be fine now

- More efficient computation of intersection with offset surface
    - Currently use brute-force appraoch
    - Improved by reducing search to a bbox
    - Improved and fixed using proximity search

- Currently use kd-tree to organize points, maybe not the best choice?


### Core Algorithm

- Correct computation of circum center for infinite faces
    - Updated with a new procedure, still needs to be checked if this is reliable
    - Should rework this, as i uses a fixed number in the computation...

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
    - Implemented both variants, has little to no difference. In one observation, the modified version needed 2 iterations less. But result looks the same.

- Labeling of faces after insertion
    - Check if we can already mark some as `outside`
