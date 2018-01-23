'''test_watershed.py - unit tests for the seeded zwatershed

'''
import unittest
import numpy as np
from seeded_zwatershed import watershed

class TestWatershed(unittest.TestCase):
    def test_00_00_nothing(self):
        segmentation = np.zeros((10, 10, 10), np.uint32)
        affinity = np.ones((3, 10, 10, 10), np.uint8) * 255
        watershed(segmentation, affinity, 0)
        self.assertTrue(np.all(segmentation == 0))

    def test_00_01_wrong_dimensions(self):
        def bad():
            segmentation = np.zeros((10, 10, 10), np.uint32)
            affinity = np.ones((10, 10, 10), np.uint8) * 255
            watershed(segmentation, affinity, 0)
            
        self.assertRaises(BaseException, bad)

    def test_00_02_wrong_size(self):
        def bad():
            segmentation = np.zeros((10, 10, 10), np.uint32)
            affinity = np.ones((2, 10, 10, 10), np.uint8) * 255
            watershed(segmentation, affinity, 0)
        
        self.assertRaises(BaseException, bad)
    
    def test_00_03_different_sizes(self):
        def bad():
            segmentation = np.zeros((10, 10, 10), np.uint32)
            affinity = np.ones((3, 20, 10, 10), np.uint8) * 255
            watershed(segmentation, affinity, 0)
        
        self.assertRaises(BaseException, bad)
    
    def test_01_01_one_object(self):
        segmentation = np.zeros((3, 3, 3), np.uint32)
        affinity = np.ones((3, 3, 3, 3), np.uint8) * 255
        segmentation[1, 1, 1] = 1
        watershed(segmentation, affinity, 0)
        self.assertTrue(np.all(segmentation == 1))
    
    def test_01_02_threshold(self):
        segmentation = np.zeros((7, 7, 7), np.uint32)
        affinity = np.ones((3, 7, 7, 7), np.uint8) * 255
        # Draw a little box around the seed
        affinity[0, 1, 1:-1, 1:-1] = 0
        affinity[0, -2, 1:-1, 1:-1] = 0
        affinity[1, 1:-1, 1, 1:-1] = 0
        affinity[1, 1:-1, -2, 1:-1] = 0
        affinity[2, 1:-1, 1:-1, 1] = 0
        affinity[2, 1:-1, 1:-1, -2] = 0
        
        segmentation[3, 3, 3] = 1
        watershed(segmentation, affinity, 1)
        mask = np.zeros((7, 7, 7), bool)
        mask[1:-2, 1:-2, 1:-2] = True
        self.assertTrue(np.all(segmentation[mask] == 1))
        self.assertTrue(np.all(segmentation[~mask] == 0))
    
    def test_02_01_two(self):
        segmentation = np.zeros((7, 7, 7), np.uint32)
        affinity = np.ones((3, 7, 7, 7), np.uint8) * 255
        segmentation[1, 3, 3] = 1
        segmentation[-2, 3, 3] = 2
        affinity[0, 3, :, :] = 0
        watershed(segmentation, affinity, 1)
        self.assertTrue(np.all(segmentation[:3] == 1))
        self.assertTrue(np.all(segmentation[3:] == 2))
    
    def test_02_02_compete(self):
        #
        # Compete for the middle. Put a hole in each of the
        # membranes, but at different levels.
        #
        segmentation = np.zeros((7, 7, 7), np.uint32)
        affinity = np.ones((3, 7, 7, 7), np.uint8) * 255
        segmentation[1, 3, 3] = 1
        segmentation[-2, 3, 3] = 2
        affinity[0, 2, :, :] = 0
        affinity[0, 4, :, :] = 0
        affinity[0, 2, 3, 3] = 5
        affinity[0, 4, 3, 3] = 6
        watershed(segmentation, affinity, 1)
        self.assertTrue(np.all(segmentation[3] == 2))