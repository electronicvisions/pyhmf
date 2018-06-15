#!/usr/bin/env python
# -*- coding: utf8 -*-

import os, sys, random, unittest, xmlrunner
import numpy
from numpy.testing import assert_equal,  assert_almost_equal, assert_array_equal

import pyhmf as pynn

class PyRandom(unittest.TestCase):
    def test_distribution_redraw(self):
        import pyhmf as pynn
        pynn.setup()

        r = pynn.NativeRNG(0)
        d = pynn.RandomDistribution("uniform", [0.0, 25.0], r, (5.0, 20.0), "redraw")
        
        data = d.next(260)
        self.assertLessEqual(max(data), 20.0)
        self.assertGreaterEqual(min(data), 5.0)
        data2 = data[data <= 5.0]
        data2 = data2[data2 >= 20]
        self.assertEqual(len(data2), 0)
        
    def test_normaldistribution(self):
        import pyhmf as pynn
        pynn.setup()
        
        mean = 42.0
        std  = 3.3
        r = pynn.NativeRNG(10)
        d = pynn.RandomDistribution("normal", [mean, std])
        
        data = d.next(100000)

        self.assertAlmostEqual(numpy.mean(data), mean, places=1)
        self.assertAlmostEqual(numpy.std(data), std, places=1)
        
    def test_distribution_clip(self):
        import pyhmf as pynn
        pynn.setup()

        r = pynn.NativeRNG(0)
        d = pynn.RandomDistribution("uniform", [0.0, 25.0], r, (12.5, 12.5), "clip")

        assert_array_equal(d.next(100), numpy.ones(100) * 12.5)

    def test_population_rset(self):
        import pyhmf as pynn
        pynn.setup()

        p = pynn.Population(123, pynn.EIF_cond_exp_isfa_ista)
        
        r1, r2 = pynn.NativeRNG(0), pynn.NativeRNG(0)
        d1 = pynn.RandomDistribution("uniform", [-55.0, -50.0])
        d2 = pynn.RandomDistribution("uniform", [-55.0, -50.0])

        p.rset("v_reset", d1)
        assert_array_equal(p.get("v_reset"), d2.next(len(p)))

    def test_binomial_distribution(self):
        import pyhmf as pynn
        pynn.setup()

        rng = pynn.NativeRNG()
        self.assertEqual(rng.next(
            distribution = "binomial", parameters = [100, 1.]), 100)

if __name__ == '__main__':
    unittest.main()
