#!/usr/bin/env python
import os, sys, random, unittest, xmlrunner
sys.path.insert(0, 'lib')

class numpy_param(unittest.TestCase):
    def test_example(self):
        import numpy
        import pyhmf_testing

        nulls  = [0.0, 0.0, 0.0, 0.0]
        target = [2.0, 0.0, 0.0, 0.0]
        a = numpy.array(nulls)
        self.assertEqual(len(a), 4)
        self.assertEqual(list(a), nulls)

        pyhmf_testing.numpyExample(a)
        self.assertEqual(len(a), 4)
        self.assertEqual(list(a), target)

    def test_projection_weights(self):
        import numpy
        from numpy.testing import assert_equal,  assert_almost_equal, assert_array_equal
        import pyhmf as pynn
        
        size = (100, 300)
        noC = size[0] * size[1]

        p1, p2 = pynn.Population(size[0], pynn.EIF_cond_exp_isfa_ista), pynn.Population(size[1], pynn.EIF_cond_exp_isfa_ista)

        prj1 = pynn.Projection(p1, p2, pynn.AllToAllConnector() )

        prj1.setWeights(4.0)
        assert_array_equal(numpy.ones(noC)  * 4.0, prj1.getWeights())
        assert_array_equal(numpy.ones(size) * 4.0, prj1.getWeights("matrix") )
        
        prj1.setWeights( numpy.ones(noC)   * 3.0 )
        assert_array_equal(numpy.ones(noC)  * 3.0, prj1.getWeights() )
        assert_array_equal(numpy.ones(size) * 3.0, prj1.getWeights("matrix") )

        prj1.setWeights( numpy.ones(size)  * 2.0 )
        assert_array_equal(numpy.ones(noC)  * 2.0, prj1.getWeights() )
        assert_array_equal(numpy.ones(size) * 2.0, prj1.getWeights("matrix") )

        prj2 = pynn.Projection(p1, p2, pynn.AllToAllConnector(delays = 2.5, weights = numpy.ones(size)))

        assert_array_equal(numpy.ones(noC)  * 2.5, prj2.getDelays() )
        assert_array_equal(numpy.ones(size) * 2.5, prj2.getDelays("matrix") )
        assert_array_equal(numpy.ones(noC)  * 1.0, prj2.getWeights() )
        assert_array_equal(numpy.ones(size) * 1.0, prj2.getWeights("matrix") )

if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(numpy_param)
    xmlrunner.XMLTestRunner().run(suite)
