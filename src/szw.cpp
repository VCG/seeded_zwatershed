/*
 * SZW.cpp - an implementation of a watershed based on affinities
 */
#include "szw.hpp"
#include <cstdint>
#include "boost/multi_array.hpp"
#include "boost/multi_array/storage_order.hpp"
#include <algorithm>
#include <vector>
#include <deque>
#include <iostream>

struct qelement_t {
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t id;
    qelement_t() {
        x = 0;
        y = 0;
        z = 0;
        id = 0;
    }
    qelement_t(const qelement_t &src) {
        x = src.x;
        y = src.y;
        z = src.z;
        id = src.id;
    }
    qelement_t(uint32_t xi, uint32_t yi, uint32_t zi, uint32_t idi) {
        x = xi;
        y = yi;
        z = zi;
        id = idi;
    }
    qelement_t & operator = (const qelement_t &src) {
        x = src.x;
        y = src.y;
        z = src.z;
        id = src.id;
        return *this;
    }
};
        
typedef std::vector<std::deque<qelement_t>> queue_t;


/*
 * Affinity is organized into a 4-d array. The first index is the
 * Z / Y / X affinity in question. This index is of length = 3.
 * The remaining indices are the Z, Y and X coordinates.
 */
class affinity_t: public boost::multi_array_ref<uint8_t, 4> {
    public:
        typedef boost::multi_array_ref<uint8_t, 4> super;
        affinity_t(PyArrayObject *a):super(
            (uint8_t *)PyArray_DATA(a), 
            boost::extents[PyArray_DIMS(a)[0]]
                          [PyArray_DIMS(a)[1]]
                          [PyArray_DIMS(a)[2]]
                          [PyArray_DIMS(a)[3]]) {
            for (size_t i=0; i < 4; i++) {
                stride_list_[i] = PyArray_STRIDE(a, i);
            }
        }
};
/*
 * The seeds and segmentation are hardcoded as uint32 for use by Python.
 */
class segmentation_t: public boost::multi_array_ref<uint32_t, 3> {
    public:
        typedef boost::multi_array_ref<uint32_t, 3> super;
        segmentation_t(PyArrayObject *a):super(
            (uint32_t *)PyArray_DATA(a), 
            boost::extents[PyArray_DIMS(a)[0]]
                         [PyArray_DIMS(a)[1]]
                         [PyArray_DIMS(a)[2]]) {
            for (size_t i=0; i < 3; i++) {
                stride_list_[i] = PyArray_STRIDE(a, i) / sizeof(uint32_t);
            }
        }
};


/*
 * init_queue()
 *
 * Initialize the queues with the segmentation seeds
 *
 * queue: 256 queues (one per level) containing the offsets
 */
void init_queue(queue_t &queue, segmentation_t &segmentation) {
    size_t z, y, x;
    auto &q255 = queue[255];
    for (z=0; z<segmentation.shape()[0]; z++) {
        for (y=0; y<segmentation.shape()[1]; y++) {
            for (x=0; x<segmentation.shape()[2]; x++) {
                /*
                 * Don't put interiors uselessly on the stack
                 */
                uint32_t e = segmentation[z][y][x];
                if ((e > 0) && (
                    ((z > 0) && (segmentation[z-1][y][x] == 0)) ||
                    ((z < segmentation.shape()[0] - 1) && (segmentation[z+1][y][x] == 0)) ||
                    ((y > 0) && (segmentation[z][y-1][x] == 0)) ||
                    ((y < segmentation.shape()[1] - 1) && (segmentation[z][y+1][x] == 0)) ||
                    ((x > 0) && (segmentation[z][y][x-1] == 0)) ||
                    ((x < segmentation.shape()[2] - 1) && (segmentation[z][y][x+1] == 0)))) {
                    q255.push_front(qelement_t(x, y, z, e));
                    /*
                     * Cross out the seed - we'll put it back later, but we
                     * need it to be zero so we know we're good to process it.
                     */
                    segmentation[z][y][x] = 0;
                }
            }
        }
    }
    std::cerr.flush();
}

void c_watershed(PyObject *psegmentation, 
               PyObject *paffinity,
               int threshold)
{
    segmentation_t segmentation=segmentation_t((PyArrayObject *)psegmentation);
    affinity_t affinity=affinity_t((PyArrayObject *)paffinity);
    queue_t queue;
    int queue_pos = 255;
    size_t xe=segmentation.shape()[2];
    size_t ye=segmentation.shape()[1];
    size_t ze=segmentation.shape()[0];
    
    queue.resize(256);
    init_queue(queue, segmentation);
    while (queue_pos >= 0) {
        if (queue[queue_pos].size() == 0) {
            queue_pos--;
            continue;
        }
        qelement_t xyze = queue[queue_pos].back();
        queue[queue_pos].pop_back();
        /*
         * If someone got to this voxel first, ignore
         */
        uint32_t e=segmentation[xyze.z][xyze.y][xyze.x];
        if (e != 0) continue;
        /*
         * Mark the voxel
         */
        segmentation[xyze.z][xyze.y][xyze.x] = e = xyze.id;
        /*
         * Do the 6 connected components
         * 1: Bounds-check
         * 2: threshold-check
         * 3: check unmarked
         * 4: enqueue
         */
        if (xyze.z > 0) {
            auto a = affinity[0][xyze.z][xyze.y][xyze.x];
            if (((int)a >= threshold) && 
                (segmentation[xyze.z-1][xyze.y][xyze.x] == 0)) {
                queue[a].push_front(qelement_t(xyze.x, xyze.y, xyze.z-1, e));
                if (a > queue_pos) queue_pos = a;
            }
        }
        if (xyze.z < ze-1) {
            auto a = affinity[0][xyze.z+1][xyze.y][xyze.x];
            if (((int)a >= threshold) && 
                (segmentation[xyze.z+1][xyze.y][xyze.x] == 0)) {
                queue[a].push_front(qelement_t(xyze.x, xyze.y, xyze.z+1, e));
                if (a > queue_pos) queue_pos = a;
            }
        }
        if (xyze.y > 0) {
            auto a = affinity[1][xyze.z][xyze.y][xyze.x];
            if (((int)a >= threshold) && 
                (segmentation[xyze.z][xyze.y-1][xyze.x] == 0)) {
                queue[a].push_front(qelement_t(xyze.x, xyze.y-1, xyze.z, e));
                if (a > queue_pos) queue_pos = a;
            }
        }
        if (xyze.y < ye-1) {
            auto a = affinity[1][xyze.z][xyze.y+1][xyze.x];
            if (((int)a >= threshold) && 
                (segmentation[xyze.z][xyze.y+1][xyze.x] == 0)) {
                queue[a].push_front(qelement_t(xyze.x, xyze.y+1, xyze.z, e));
                if (a > queue_pos) queue_pos = a;
            }
        }
        if (xyze.x > 0) {
            auto a = affinity[2][xyze.z][xyze.y][xyze.x];
            if (((int)a >= threshold) && 
                (segmentation[xyze.z][xyze.y][xyze.x-1] == 0)) {
                queue[a].push_front(qelement_t(xyze.x-1, xyze.y, xyze.z, e));
                if (a > queue_pos) queue_pos = a;
            }
        }
        if (xyze.x < xe-1) {
            auto a = affinity[2][xyze.z][xyze.y][xyze.x+1];
            if (((int)a >= threshold) && 
                (segmentation[xyze.z][xyze.y][xyze.x+1] == 0)) {
                queue[a].push_front(qelement_t(xyze.x+1, xyze.y, xyze.z, e));
                if (a > queue_pos) queue_pos = a;
            }
        }
    }
}
