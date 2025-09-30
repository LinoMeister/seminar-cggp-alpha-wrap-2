# Alpha Wrapping with an Offset in 2D


## TODO

- Correct implementation of intersection with offset surface
    - Currently we just return closest point, which means the generated triangles will always intersect the input and we are not able to carve any faces
    - Updated to manual computation of intersection with circle around closest input point
- Correct computation of circum center for infinite faces
    - Updated with a new procedure, still needs to be checked if this is reliable