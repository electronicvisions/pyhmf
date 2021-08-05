#!/usr/bin/env python

"""PyNN 0.8 tests based on the changelog between 0.7 and 0.8

See issue #2045 for details.
"""

import unittest
import numpy
import pyNN
import pyhmf


class TestAPIChanges(unittest.TestCase):
    API_VERSION=pyhmf.__version__[:3]
    SUPPORTED_API_VERSIONS = ["0.7", "0.8"]

    def setUp(self):
        pyhmf.setup()

    def tearDown(self):
        pyhmf.end()

    def test_version(self):
        self.assertTrue(self.API_VERSION in self.SUPPORTED_API_VERSIONS,
            "Present version {} not among supported: {}.".format(
            self.API_VERSION, ", ".join(self.SUPPORTED_API_VERSIONS)))

    def test_issue2046(self):
        if self.API_VERSION in ("0.7", "0.8"):
            # PyNN 0.7 style, deprecated but still supported in 0.8
            p = pyhmf.Population(1000, pyhmf.IF_cond_exp, {'tau_m': 12.0, 'cm': 0.8})

        if self.API_VERSION == "0.8":
            p = pyhmf.Population(1000, pyhmf.IF_cond_exp(**{'tau_m': 12.0, 'cm': 0.8}))

            cell_type = pyhmf.IF_cond_exp(tau_m=12.0, cm=0.8)
            p = pyhmf.Population(1000, cell_type)

    @unittest.skip("too much jenkins noise")
    def test_issue2047(self):
        arr = numpy.zeros(1000)
        rand_distr = numpy.zeros(1000)
        if self.API_VERSION == "0.8":
            p = pyhmf.Population(1000, pyhmf.IF_cond_exp(**{'tau_m': 12.0, 'cm': 0.8}))
            p.set(tau_m=20.0)
            p.set(tau_m=20.0, v_rest=-65)
            p.tset("i_offset", arr)  # deprecated
            p.rset("tau_m", rand_distr)  # deprecated
            p.set(i_offset=arr)
            p.set(tau_m=rand_distr)
        elif self.API_VERSION == "0.7":
            p = pyhmf.Population(1000, pyhmf.IF_cond_exp, {'tau_m': 12.0, 'cm': 0.8})
            p.set({"tau_m": 20.0})
            p.set({"tau_m": 20.0, "v_rest": -65})
            p.tset("i_offset", arr)
            p.rset("tau_m", rand_distr)

    @unittest.skip("too much jenkins noise")
    def test_issue2048(self):
        if self.API_VERSION == "0.8":
            def generate_spike_times(i_range):
                return [pyNN.parameters.Sequence(numpy.add.accumulate(numpy.random.exponential(10.0, size=10))) for i in i_range]
            p = pyhmf.Population(30, pyhmf.SpikeSourceArray(spike_times=generate_spike_times))
        elif self.API_VERSION == "0.7":
            def generate_spike_times(i_range):
                return [numpy.add.accumulate(numpy.random.exponential(10.0, size=10)) for i in i_range]
            p = pyhmf.Population(30, pyhmf.SpikeSourceArray, {'spike_times': generate_spike_times(list(range(30)))})

    def test_issue2049(self):
        p = pyhmf.Population(1000, pyhmf.EIF_cond_exp_isfa_ista)
        if self.API_VERSION == "0.8":
            p.record('spikes')
            p.record('v')
            # for AdEx population
            p.record(['gsyn_exc', 'gsyn_inh'])
            p.record(['spikes', 'v', 'w', 'gsyn_exc', 'gsyn_inh'])
            p.record_v()  # deprecated
            p.record_gsyn()  # deprecated
        elif self.API_VERSION == "0.7":
            p.record()
            p.record_v()
            p.record_gsyn()

    def test_issue2051(self):
        p1 = pyhmf.Population(1000, pyhmf.EIF_cond_exp_isfa_ista)
        p2 = pyhmf.Population(1000, pyhmf.EIF_cond_exp_isfa_ista)
        if self.API_VERSION == "0.8":
            prj = pyhmf.Projection(p1, p2, pyhmf.AllToAllConnector(), pyhmf.StaticSynapse(weight=0.05, delay=0.5))
            params = {'U': 0.04, 'tau_rec': 100.0, 'tau_facil': 1000.0, 'weight': 0.01}
            facilitating = pyhmf.TsodyksMarkramSynapse(**params)
            prj = pyhmf.Projection(p1, p2, pyhmf.FixedProbabilityConnector(p_connect=0.1), synapse_type=facilitating)
        elif self.API_VERSION == "0.7":
            prj = pyhmf.Projection(p1, p2, pyhmf.AllToAllConnector(weights=0.05, delays=0.5))
            params = {'U': 0.04, 'tau_rec': 100.0, 'tau_facil': 1000.0}
            facilitating = pyhmf.SynapseDynamics(fast=pyhmf.TsodyksMarkramMechanism(**params))
            prj = pyhmf.Projection(p1, p2, pyhmf.FixedProbabilityConnector(p_connect=0.1, weights=0.01), synapse_dynamics=facilitating)

    @unittest.skip("too much jenkins noise")
    def test_issue2053(self):
        p1 = pyhmf.Population(1000, pyhmf.EIF_cond_exp_isfa_ista)
        p2 = pyhmf.Population(1000, pyhmf.EIF_cond_exp_isfa_ista)
        rand_distr = numpy.zeros(1000)
        if self.API_VERSION == "0.8":
            prj = pyhmf.Projection(p1, p2, pyhmf.AllToAllConnector(), pyhmf.StaticSynapse(weight=0.05, delay=0.5))
            prj.get('weight', format='list', with_address=False)
            prj.set(delay=rand_distr)
            prj.set(tau_rec=50.0)
            prj.save('weight', 'exc_weights.txt', format='array')
            prj.save('all', 'exc_conn.txt', format='list')
            weights, delays = prj.get(['weight', 'delay'], format='array')
            prj.set(weight=rand_distr, delay=0.4)
        elif self.API_VERSION == "0.7":
            prj = pyhmf.Projection(p1, p2, pyhmf.AllToAllConnector(weights=0.05, delays=0.5))
            prj.getWeights(format='list')
            prj.randomizeDelays(rand_distr)
            prj.setSynapseDynamics('tau_rec', 50.0)
            prj.printWeights('exc_weights.txt', format='array')
            prj.saveConnections('exc_conn.txt')
            weights, delays = prj.getWeights('array'), prj.getDelays('array')
            prj.randomizeWeights(rand_distr); prj.setDelays(0.4)

if __name__ == '__main__':
    unittest.main()
