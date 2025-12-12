# Experiments


>[!IMPORTANT]
>In this report two different experiments are featured. *Alternative* traversability refers to just a small modification of the vanilla traversability criteria. This is just a small curiosity that I wanted to explore very briefly. Whereas *adaptive* traversability is something different. Adaptive traversability describes a more substantial change to how traversability is defined and is the main focus of the project. Just don't get confused.

## Alternative Traversability Criteria

In the vanilla algorithm a gate is traversable if the radius of its minimum Delaunay circle  is $\geq \alpha$. This definition can be motivated with the idea of using a spherical spoon of radius $\alpha$ to carve out the inside cell without touching any of the vertices.

The minimum Delaunay circle corresponds to one of the following 3 cases:

1. It is the smallest sphere through the gate vertices
2. It is the circumcircle of the *inside* cell
3. It is the circumcircle of the *outside* cell

## Adaptive Traversability

### Traversability Parameters

#### Target Length

There are a couple of parameters to tune the adaptive traversability methods. Most importantly is the **target length** parameter, which determines has the following roles:

- Deviation based: Target length of the subsegments used in the deviation computation
- Intersection based: Target length for the spacing between the sample points

For both methods this parameter allows us to control how large a hole need to be for it to be considered traversable. A large target length results in a less detailed evaluation of a gate, where small features are less likely to make a difference on traversability. A small target length allows us to carve smaller holes, but makes checking traversability more expensive.

The target length parameter is set equal to the specified $\alpha$ parameter. When comparing the adaptive methods to using a constant alpha, all the methods use the same $\alpha$ (i.e., target length).

#### Other Parameters

The remaining parameters cannot be specified through the command line tool. These parameters are stored in the `DeviationBasedParams` and `IntersectionBasedParams` structs, located in  `/include/alpha_wrap_2/traversability.h`  See the source code comments for a detailed explanation of their purpose. For the experiments the values specified in `main.cpp` were used.


### Comparison

The 3 methods (global alpha, deviation-based and intersection-based) were compared on a small dataset of just 8 example inputs. The comparison below shows the obtained output using different parameters on just one of the example inputs. The full experimental data for all 8 examples is available in the supplementary material.

#### $\alpha=0.01$ and $\epsilon=0.01$
#### $\alpha=0.01$ and $\epsilon=0.05$
#### $\alpha=0.05$ and $\epsilon=0.01$
#### $\alpha=0.05$ and $\epsilon=0.05$

### Runtime

<table>
  <tr>
    <td><img src="images/comparison_a_0.01_o_0.01/runtime_breakdown_stacked.png" width="500"></td>
    <td><img src="images/comparison_a_0.01_o_0.05/runtime_breakdown_stacked.png"  width="500"></td>
  </tr>
  <tr>
    <td><img src="images/comparison_a_0.05_o_0.01/runtime_breakdown_stacked.png"  width="500"></td>
    <td><img src="images/comparison_a_0.05_o_0.05/runtime_breakdown_stacked.png"  width="500"></td>
  </tr>
</table>

### Discussion

#### Observations

In these experiments we can observe the following:

- The adaptive methods are able to more economically insert vertices
- Both adaptive methods spend less time for checking R1 and R2, but spend much more time on checking the traversability of gates.
- Intersection-based traversability more consistently produces desirable results and usually terminates faster than deviation-based traversability. In particular the deviation-based method fails to produce a satisfying result when increasing the offset.

#### Considerations

This comparison should however merely give a rough idea of what sort of results can be produced by these methods. It does *not* necessarily offer a complete or fair comparison of these methods. The following points should be mentioned to add some context to the results:

- To make the result in some way comparable, all methods used the same value for  $\alpha$ or target length. However, better results might be achieved by choosing  the remaining traversability parameters  of the adaptive methods differently. 
- The implementation of the procedure for updating the queue can be made more efficient. Currently the queue is cleared and rebuilt by checking every facet in the triangulation. The adaptive methods would especially benefit from a more efficient implementation, as checking gates for traversability is what currently takes up most of their runtime.
- The methods were compared on a small dataset of rather simple example inputs which were all described by a  *dense* point cloud. Behavior on "messy" real-world data might be different. A general concern of the alpha wrapping method is to ensure that we don't traverse through hole in the offset surface and start carving out the entire inside volume of the input geometry. 

#### Conclusion

Intersection-based traversability appears to be a viable approach for adaptive traversability. From these experiments it cannot be concluded yet if it is able to produce satisfyable results in a 3D scenario and do so within reasonable runtime. This could however be tested in terms of future work, as it is fairly simple to extend this modification to the 3D algorithm. 