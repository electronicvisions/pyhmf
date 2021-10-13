#! /usr/bin/python
# -*- coding: utf-8 -*-

import tempfile
import random
import unittest
import numpy.testing

import pyhmf
import pyNN.nest as pynn

class FileTest(object):
    file_type = None

    def test_projection(self):

        path = tempfile.mkstemp()[1]

        size_a = random.randint(1, 100)
        size_b = random.randint(1, 100)

        dist = pyhmf.RandomDistribution(rng=pyhmf.NativeRNG(1337))

        conn_pyhmf = pyhmf.AllToAllConnector(weights=dist, delays=42)
        proj_pyhmf = pyhmf.Projection(
                pyhmf.Population(size_a, pyhmf.IF_cond_exp),
                pyhmf.Population(size_b, pyhmf.IF_cond_exp),
                conn_pyhmf)
        
        proj_pyhmf.saveConnections(getattr(pyhmf, self.file_type)(path, 'wb'))

        conn_pynn = pynn.FromFileConnector(getattr(pynn.recording.files, self.file_type)(path))
        proj_pynn = pynn.Projection(
                pynn.Population(size_a, pynn.IF_cond_exp()),
                pynn.Population(size_b, pynn.IF_cond_exp()),
                conn_pynn)

        numpy.testing.assert_equal(proj_pyhmf.getWeights(format='array'), proj_pynn.getWeights(format='array'))


class StandardTextFileTest(FileTest, unittest.TestCase):
    file_type = 'StandardTextFile'

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)
        FileTest.__init__(self)


class NumpyFileTest(FileTest, unittest.TestCase):
    file_type = 'NumpyBinaryFile'

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)
        FileTest.__init__(self)


class PickleFileTest(FileTest, unittest.TestCase):
    file_type = 'PickleFile'

    def __init__(self, *args, **kwargs):

        unittest.TestCase.__init__(self, *args, **kwargs)
        FileTest.__init__(self)


if __name__ == '__main__':
    unittest.main()
