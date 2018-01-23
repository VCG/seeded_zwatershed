/*
 * SZW.hpp - the interface for the seeded watershed
 */
#ifndef SZW_HPP
#define SZW_HPP
#include "numpy/arrayobject.h"

/*
 * watershed()
 *    segmentation: on input, this has seeds with separate integer values
 *                  for each. On output, has the watershed segmentation.
 *    affinity: the affinities between adjacent voxels. The size of the
 *              array must match that of the segmentation array.
 *    threshold: No connections will be made for affinities greater than
 *               the threshold.
 */
extern void c_watershed(PyObject *segmentation, 
               PyObject *affinity,
               int threshold);
#endif