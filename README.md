# Seeded Affinity Watershed

This package implements a seeded watershed using affinity predictions to
connect components. Affinity predictions relate two voxels via an affinity
score - if the score is low, the voxels should be connected and if the
score is high, the voxels should not be connected.

The watershed proceeds from the initial markers, propagating the marker label
to the 6-connected (z up, z down, y up, y down and x up, x down) neighborhood.
The propagation first follows the neighbors connected by the lowest affinity

The watershed takes three arguments:

* **segmentation**: a 3 dimensional array. Initially, the array holds the
    markers for the watershed and upon completion, it holds the final
    segmentation.

* **affinity**: a 4 dimensional array with the first dimension having size=3.
    The first dimension selects the z-affinity, y-affinity or x-affinity.
    The remaining dimensions give the coordinate of the lower of the two
    voxels to be connected. For instance, to find the affinity between
    voxels, *z*, *y*, *x* and *z+1*, *y*, *x*, you reference
    *affinity[0, z, y, x]*.

* **threshold**: no connections will be made for affinities whose values
    are greater than this threshold.

## Installation

Installation requires Numpy and Cython in your environment as Python 
dependencies. It also requires Boost on your include path to compile the C++
portion of the package.

For instance, to install using Anaconda:

```
> conda create -n myenv numpy cython boost
> source activate myenv
> git clone https://github.com/vcg/seeded_zwatershed
> pip install -e seeded_watershed
```
