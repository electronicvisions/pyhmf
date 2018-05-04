#!/usr/bin/env python
# -*- coding: utf8 -*-

import os
import sys
import unittest
import nose
import random
import numpy
import numpy.testing
import argparse
import copy
from utils import repeat, fails, parametrize
import pyhmf

from pyNN.random import AbstractRNG
import pyNN.nest as pynn
pynn.setup()

REPEAT = os.getenv('TEST_REPEAT') in ['true', '1']

class NativeRNGProxy(AbstractRNG):
    
    def __init__(self, *args, **kwargs):
        AbstractRNG.__init__(self)
        self._impl = pyhmf.NativeRNG(*args, **kwargs)

    def next(self, n=1, distribution='uniform', parameters=[], **kwargs):
        return self._impl.next(n, distribution, list(parameters))

    def permutation(self, l):
        return self._impl.permutation(list(l))

    def binomial(self, *args, **kwargs):
        return self._impl.binomial(*args, **kwargs)

    def exponential(self, *args, **kwargs):
        return self._impl.exponential(*args, **kwargs)

    def normal(self, *args, **kwargs):
        return self._impl.normal(*args, **kwargs)

    def poisson(self, *args, **kwargs):
        return self._impl.poisson(*args, **kwargs)

    def randint(self, *args, **kwargs):
        return self._impl.randint(*args, **kwargs)

    def uniform(self, *args, **kwargs):
        return self._impl.uniform(*args, **kwargs)


pynn.NativeRNG = NativeRNGProxy


class ConnectorTest(object):
    connector = None

    def __init__(self, *args, **kwargs):

        self.random = pyhmf.NativeRNG()
    

    def construct_with_backend(self, backend, size, weights, seed=1337, c_args={}):

        c_args['weights'] = weights
        
        pre = backend.Population(size, backend.IF_cond_exp)
        post = backend.Population(size, backend.IF_cond_exp)

        connector = getattr(backend, self.connector)(**c_args)
        projection = backend.Projection(pre, post, connector, rng=backend.NativeRNG(seed))

        return projection.getWeights(format='array').astype(numpy.float32)


    def test_init_value(self):

        size = random.randint(2, 10)**2
        weights = random.random()

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights),
                self.construct_with_backend(pyhmf, size, weights)
                )


    def test_init_vector(self):

        size = random.randint(2, 10)**2
        size = 2
        weights = numpy.array(self.random.next(size**2))

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights),
                self.construct_with_backend(pyhmf, size, weights)
                )


    def test_init_random(self):

        size = random.randint(2, 10)**2
        seed = 1337

        pynn_dist = pynn.RandomDistribution(
                distribution='uniform',
                rng = NativeRNGProxy(seed)
                )
        pyhmf_dist = pyhmf.RandomDistribution(
                distribution='uniform',
                rng = pyhmf.NativeRNG(seed)
                )

        numpy.testing.assert_almost_equal(
                self.construct_with_backend(pynn, size, pynn_dist),
                self.construct_with_backend(pyhmf, size, pyhmf_dist)
                )


class AllToAllConnectorTest(ConnectorTest, unittest.TestCase):

    connector = 'AllToAllConnector'

    def __init__(self, *args, **kwargs):

        ConnectorTest.__init__(self)
        unittest.TestCase.__init__(self, *args, **kwargs)

    
    @fails("Fails due to PyNN issue #243")
    def test_init_vector(self):
        return ConnectorTest.test_init_vector(self)


class OneToOneConnectorTest(ConnectorTest, unittest.TestCase):

    connector = 'OneToOneConnector'

    def __init__(self, *args, **kwargs):

        ConnectorTest.__init__(self)
        unittest.TestCase.__init__(self, *args, **kwargs)


class FixedProbabilityConnectorTest(ConnectorTest, unittest.TestCase):

    connector = 'FixedProbabilityConnector'

    def __init__(self, *args, **kwargs):

        ConnectorTest.__init__(self)
        unittest.TestCase.__init__(self, *args, **kwargs)


    def test_init_value(self):

        size = random.randint(2, 10)**2
        weights = random.random()

        seed = 1337
        c_args = {'p_connect': random.random()}

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights, c_args=c_args, seed=seed),
                self.construct_with_backend(pyhmf, size, weights, c_args=c_args, seed=seed)
                )


    @fails("Fails due to PyNN issue #243")
    def test_init_vector(self):

        size = random.randint(2, 10)**2
        weights = numpy.array(self.random.next(size**2))

        seed = 1337
        c_args = {'p_connect': random.random()}

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights, c_args=c_args, seed=seed),
                self.construct_with_backend(pyhmf, size, weights, c_args=c_args, seed=seed)
                )


    @fails("Fails due to strange weights initialization in PyNN")
    def test_init_random(self):

        size = random.randint(2, 10)**2
        size = 4
        seed = 1337

        c_args = {'p_connect': random.random()}

        pynn_dist = pynn.RandomDistribution(
                distribution='uniform',
                rng = NativeRNGProxy(seed)
                )
        pyhmf_dist = pyhmf.RandomDistribution(
                distribution='uniform',
                rng = pyhmf.NativeRNG(seed)
                )

        numpy.testing.assert_almost_equal(
                self.construct_with_backend(pynn, size, pynn_dist, c_args=c_args, seed=seed),
                self.construct_with_backend(pyhmf, size, pyhmf_dist, c_args=c_args, seed=seed)
                )


class FixedNumberPreConnectorTest(ConnectorTest, unittest.TestCase):

    connector = 'FixedNumberPreConnector'

    def __init__(self, *args, **kwargs):

        ConnectorTest.__init__(self)
        unittest.TestCase.__init__(self, *args, **kwargs)


    def test_init_value(self):

        size = random.randint(2, 10)**2
        weights = random.random()

        seed = 1337
        c_args = {'n': random.randint(0, size)}

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights, c_args=c_args, seed=seed),
                self.construct_with_backend(pyhmf, size, weights, c_args=c_args, seed=seed)
                )


    def test_init_vector(self):

        size = random.randint(2, 10)**2
        weights = numpy.array(self.random.next(size**2))

        seed = 1337
        c_args = {'n': random.randint(0, size)}

        print c_args, size

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights, c_args=c_args, seed=seed)*0,
                self.construct_with_backend(pyhmf, size, weights, c_args=c_args, seed=seed)*0
                )


    def test_init_random(self):

        size = random.randint(2, 10)**2
        seed = 1337

        n = random.randint(0, size)
        c_args = {'n': n}

        pynn_dist = pynn.RandomDistribution(
                distribution='uniform',
                rng = NativeRNGProxy(seed)
                )
        pyhmf_dist = pyhmf.RandomDistribution(
                distribution='uniform',
                rng = pyhmf.NativeRNG(seed)
                )

        numpy.testing.assert_almost_equal(
                self.construct_with_backend(pynn, size, pynn_dist, c_args=c_args, seed=seed)*0,
                self.construct_with_backend(pyhmf, size, pyhmf_dist, c_args=c_args, seed=seed)*0
                )


class FixedNumberPostConnectorTest(FixedNumberPreConnectorTest):
    connector = 'FixedNumberPostConnector'

    @fails("Fails due to #256 in PyNN")
    def test_init_random(self):
        FixedNumberPreConnectorTest.test_init_random(self)
    
    @fails("Fails due to #261 in PyNN")
    def test_init_vector(self):
        FixedNumberPreConnectorTest.test_init_vector(self)

class DistanceDependentProbabilityConnectorTest(unittest.TestCase):

    d_expr_modes = {'small_world':"d<0.5",
                    'exp': "exp(-d)",
                    'exp_abs': "exp(-abs(d))",
                    'linear': "d*0.4",
                    'gaussian': "1.*exp(-(d**2)/(2*(0.2**2)))"}

    def __init__(self, *args, **kwargs):
        self.random = pyhmf.NativeRNG()
        unittest.TestCase.__init__(self, *args, **kwargs)

    def construct_with_backend(self, backend, size, weights, seed=1337, c_args={}):

        conn_args = copy.copy(c_args)
        conn_args['weights'] = weights

        if backend.__name__== 'pyhmf':
            grid2d = backend.Grid2D(dx=1./numpy.sqrt(size),dy=1./numpy.sqrt(size))
        else:
            grid2d = backend.space.Grid2D(dx=1./numpy.sqrt(size),dy=1./numpy.sqrt(size))
        pre = backend.Population(size, backend.IF_cond_exp, structure=grid2d)
        post = backend.Population(size, backend.IF_cond_exp, structure=grid2d)

        conn_args['space'] = backend.Space('xy')

        connector = backend.DistanceDependentProbabilityConnector(**conn_args)
        projection = backend.Projection(pre, post, connector, rng=backend.NativeRNG(seed))

        return projection.getWeights(format='array').astype(numpy.float32)

    @parametrize(d_expr_modes.keys())
    def test_d_expression(self, d_expr_mode):

        size = random.randint(2, 10)**2
        weights = random.random()

        seed = 1337
        c_args = {'d_expression': self.d_expr_modes[d_expr_mode]}

        numpy.testing.assert_equal(
                self.construct_with_backend(pynn, size, weights, c_args=c_args, seed=seed),
                self.construct_with_backend(pyhmf, size, weights, c_args=c_args, seed=seed)
                )

    @unittest.expectedFailure # cf. Issue #2261
    def test_n_connections(self):
        size = 100
        weights = random.random()
        seed = 1337
        n_connections = 43 # nr of efferent connections
        c_args = {'d_expression': self.d_expr_modes['gaussian'],
                'n_connections': n_connections
                }

        w_pyhmf = self.construct_with_backend(pyhmf, size, weights, c_args=c_args, seed=seed)
        for row in w_pyhmf:
            self.assertEqual(n_connections, numpy.count_nonzero(numpy.isfinite(row)))
        # comparision to nest does not make any sense here, as the
        # n_connections behaviour is not well specified in PyNN, cf. #2261

if __name__ == '__main__':
    unittest.main()
