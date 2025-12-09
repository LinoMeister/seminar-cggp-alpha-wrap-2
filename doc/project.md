## Alpha Wrapping with an Offset

Alpha wrapping with an offset is an algorithm presented in a [paper](https://inria.hal.science/hal-03688637) form 2022, authored by Portaneri et al. Alongside the paper, an implementation of the algorithm was made available through [CGAL](https://doc.cgal.org/latest/Alpha_wrap_3/index.html).

>[!NOTE] 
>**Abstract**: Given an input 3D geometry such as a triangle soup or a point set, we address the problem of generating a watertight and orientable surface triangle mesh that strictly encloses the input. The output mesh is obtained by greedily refining and carving a 3D Delaunay triangulation on an offset surface of the input, while carving with empty balls of radius alpha. The proposed algorithm is controlled via two user-defined parameters: alpha and offset. Alpha controls the size of cavities or holes that cannot be traversed during carving, while offset controls the distance between the vertices of the output mesh and the input. Our algorithm is guaranteed to terminate and to yield a valid and strictly enclosing mesh, even for defect-laden inputs. Genericity is achieved using an abstract interface probing the input, enabling any geometry to be used, provided a few basic geometric queries can be answered. We benchmark the algorithm on large public datasets such as Thingi10k, and compare it to state-of-the-art approaches in terms of robustness, approximation, output complexity, speed, and peak memory consumption. Our implementation is available through the CGAL library.

## Project Overview

As part of this project I implemented a version of this algorithm that works on 2D scenarios and is limited to point cloud inputs. This implementation is *not* based on the CGAL alpha wrapping implementation, however it does use structures and algorithms from the CGAL library (e.g., Delaunay triangulation or KD-tree).

### Adaptive Traversability

A *gate* is a facet of the triangulation where one adjacent cell is marked *inside* and the other adjacent cell is marked *outside*. The algorithm can perform a refinement or carving operation at a gate. To make sure that gates are not processed indefinitely, the algorithm only considers gates that are deemed *traversable*. A gate is traversable if the radius of the minimum Delaunay ball through the gate is $\geq\alpha$. 

The alpha wrapping algorithm described in the paper uses a global $\alpha$ parameter to control the granularity of the reconstructed surface. Using a large $\alpha$ results in some input details not being captured well, whereas choosing it small will often lead to the unnecessary insertion of vertices in flat regions. 

IMAGE

As part of the project I experimented with adaptive traversability criteria that determine the traversability of a gate based on the local input geometry. Two different approaches are implemented in this repository: 

- *Deviation-based traversability* : This was a first attempt at approaching adaptive traversability. The method is admittedly rather messy and does suffer from some issues making it hard to achieve good results.
- *Intersection-based traversability*: A simpler and more elegant method that avoids many of the issues of the deviation-based approach. 

>[!NOTE] 
>Below is a brief textual description of these methods, also see p.31-34 in the slides of the supplementary material for a visual explanation.

#### Deviation-Based Traversability

To determine if a gate $f$ is traversable, the following procedure is followed:
- Construct an axis-aligned bounding box around the $f$
- Find all the input points $p_{i}$ that are located within this bounding box
- Measure the mean-squared distance from these points to $f$

$$
\delta=\frac{1}{N}\sum_{i=1}^N d(f,p_{i})^2
$$

From the measured deviation $\delta$ we then need to obtain a value for $\alpha$. In my implementation this was achieved by constructing a normalized deviation $\tilde{\delta}\in[0,1]$.

From this normalized deviation value, $\alpha$ is obtained as:
$$
\alpha = (1-\tilde{\delta})\alpha_{\text{max}} + \tilde{\delta} \alpha_{\text{min}}
$$
where $\alpha_{\text{min}}$ is set equal to the value specified by the user through  `--alpha` and $\alpha_{\text{max}}$ is a fixed value specified in the traversabiliy parameters (In my experiments it was simply set to  `200`, but it could instead also be tied to some reference length (e..g, bounding box diagonal length) ). 

##### Subsegment Improvement

The approach outlined above often finds a small deviation in 'cave-like' areas, because many of the relevant input features are not captures by the bounding box of the gate. As an attempt to solve this problem, the deviation is not computed on the entire gate, but the gate is split up into smaller segments $f_{j}$ for which the deviation $\delta_{j}$ is computed. These segments are constructed to have a length of $\approx L_{\text{target}}$ where the target length is specified as a global parameter. The deviations are then aggregated to a single value for the segment.
$$
\tilde{\delta} =\max_{j}\{ \tilde{\delta} \}
$$
In 'cave-like' areas we are thus more likely to end up with a subsegment whose bounding box only contains very few or no input points. For such segments we artificially assign a large deviation ($\tilde{\delta}=1$), which will consequently lead to a large deviation for $f$, making the gate more likely to be traversable.

#### Intersection-Based Traversability

To determine if a gate $f$ is traversable, it is sampled at points $q_{i}$. In my implementation the points are samples equidistantly such that the distance between the points is $\approx L_{\text{target}}$. Then we consider the line segments $(q_{i}, q_{i} + \lambda n)$ where $n$ is the normal vector of $f$ and $\lambda$ is a user-defined global parameter. For each such segment we check if it intersects with the offset surface. If at any point there is no intersection (i.e., the distance between the point and the offset surface is $>\lambda$), we mark the gate as traversable.

