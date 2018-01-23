'''_watershed.pyx - Cython binding of seeded affinity watershed

'''

cimport numpy as np
from cpython.object cimport PyObject
cdef extern from "szw.hpp" nogil:
    void c_watershed(PyObject *psegmentation, PyObject *pAffinity, int threshold)
    

def watershed(np.ndarray[dtype=np.uint32_t, ndim=3] segmentation,
              np.ndarray[dtype=np.uint8_t, ndim=4] affinity, threshold):
    cdef:
        int ithreshold = threshold
        PyObject *psegmentation=<PyObject *>segmentation
        PyObject *paffinity=<PyObject *>affinity

    assert affinity.shape[0] == 3, \
        "The affinity's first index should be 0, 1, and 2"
    assert segmentation.shape[0] == affinity.shape[1], \
        "The affinity's Z shape (%d) must be the same as the segmentation (%d)"\
        % (affinity.shape[1], segmentation.shape[0])
    assert segmentation.shape[1] == affinity.shape[2], \
        "The affinity's Y shape (%d) must be the same as the segmentation (%d)"\
        % (affinity.shape[2], segmentation.shape[1])
    assert segmentation.shape[2] == affinity.shape[3], \
        "The affinity's X shape (%d) must be the same as the segmentation (%d)"\
        % (affinity.shape[3], segmentation.shape[2])

    with nogil:
        c_watershed(psegmentation, paffinity, ithreshold)