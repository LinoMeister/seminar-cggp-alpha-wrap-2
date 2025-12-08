# Experiments

## Simple Example Result


## Adaptive Traversability

### Traversability Parameters

#### Target Length

There are a couple of parameters to tune the adaptive traversability methods. Most importantly is the **target length** parameter, which determines has the following roles:

- Deviation based: Target length of the subsegments used in the deviation computation
- Intersection based: Target length for the spacing between the sample points

For both methods this parameter allows us to control how large a hole need to be for it to be considered traversable. A large target length results in a less detailed evaluation of a gate, where small features are less likely to make a difference on traversability. A small target length allows us to carve smaller holes, but makes checking traversability more expensive.

The target length parameter is set equal to the specified $\alpha$ parameter. When comparing the adaptive methods to using a constant alpha, all the methods use the same $\alpha$ (i.e., target length).

#### Other Parameters

The remaining parameters cannot be specified through the command line tool. For my experiments, the parameters were set as the default values specified in `DeviationBasedParams` and `IntersectionBasedParams`, located in  `/include/alpha_wrap_2/traversability.h`. 

See the source code comments for a detailed explanation of their purpose. 


### Comparison



### Discussion

This comparison should merely give a rough idea of what sort of results can be produced by these methods. It does *not* offer a complete or fair comparison of these methods. 

- To make the result in some way comparable, all methods used the same value for  $\alpha$ or target length. However, different choices for the remaining traversability parameters of the adaptive methods might exist, that result in "better" outputs or reduced runtime.
- The implementation of the procedure for updating the queue can be made more efficient. Currently the queue is cleared and rebuilt by checking every facet in the triangulation. The adaptive methods would especially benefit from a more efficient implementation, as checking gates for traversability is what currently takes up most of their runtime.
- The methods were compared on a small dataset of rather simple example inputs which were all described by a  *dense* point cloud. Behavior on "messy" real-world data might be different. A general concern of the alpha wrapping method is to ensure that we don't traverse through hole in the offset surface and start carving out the entire inside volume of the input geometry. 