#! /usr/bin/python
# -*- coding: utf-8 -*-

import unittest

import numpy as np

import pyhmf
import pyNN.nest as pynn

class SpaceTest(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)


    def test_distances(self):

        A = np.random.rand(3, 100)
        B = np.random.rand(3, 100)

        np.testing.assert_equal(
                pyhmf.Space().distances(A, B),
                pynn.Space().distances(A, B)
                )
    
    
    def test_distances_scale(self):

        A = np.random.rand(3, 100)
        B = np.random.rand(3, 100)

        np.testing.assert_equal(
                pyhmf.Space(scale_factor=42.).distances(A, B),
                pynn.Space(scale_factor=42.).distances(A, B)
                )
   

    def test_distances_offset(self):

        A = np.random.rand(3, 100)
        B = np.random.rand(3, 100)
        
        offset = np.random.rand(3, 1)

        np.testing.assert_equal(
                pyhmf.Space(offset=offset).distances(A, B),
                pynn.Space(offset=offset).distances(A, B)
                )


    def test_distances_scale_offset(self):

        A = np.random.rand(3, 100)
        B = np.random.rand(3, 100)

        offset = np.random.rand(3, 1)
        
        np.testing.assert_equal(
                pyhmf.Space(scale_factor=42., offset=offset).distances(A, B),
                pynn.Space(scale_factor=42., offset=offset).distances(A, B)
                )
   

    def test_distances_boundaries(self):

        A = np.random.rand(3, 100)
        B = np.random.rand(3, 100)

        np.testing.assert_equal(
                pyhmf.Space(periodic_boundaries=((0, 1), None, (0, 1))).distances(A, B),
                pynn.Space(periodic_boundaries=((0, 1), None, (0, 1))).distances(A, B)
                )
   

    def test_distances_boundaries_scale_offset(self):

        A = np.random.rand(3, 100)
        B = np.random.rand(3, 100)

        offset = np.random.rand(3, 1)
        
        np.testing.assert_equal(
                pyhmf.Space(scale_factor=42., offset=offset, periodic_boundaries=((0, 42.), None, (0, 42.))).distances(A, B),
                pynn.Space(scale_factor=42., offset=offset, periodic_boundaries=((0, 42.), None, (0, 42.))).distances(A, B)
                )


if __name__ == '__main__':
    unittest.main()
