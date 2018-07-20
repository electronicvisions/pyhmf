#!/usr/bin/env python
# -*- coding: utf8 -*-

import os, sys, random, unittest, xmlrunner
import pyhmf as pynn


def pymarocco_available():
    try:
        import pymarocco
    except ImportError:
        return False
    return True

@unittest.skipIf(not pymarocco_available(), "Test requires pymarocco")
class Assembly(unittest.TestCase):
    def test_Constructor(self):
        import pymarocco

        marocco = pymarocco.PyMarocco()
        marocco.calib_backend = pymarocco.PyMarocco.CalibBackend.Default

        pynn.setup(marocco=marocco)

        NPOP = 100
        popus = [pynn.Population(random.randint(10,100), pynn.IF_cond_exp) for x in range(0,NPOP)]
        a = pynn.Assembly(*popus)

        size = sum( [ len(p) for p in popus ] )
        self.assertEqual(len(a), size)

        pynn.run(1000)


    def test_Addition(self):
        import pymarocco

        marocco = pymarocco.PyMarocco()
        marocco.calib_backend = pymarocco.PyMarocco.CalibBackend.Default

        pynn.setup(marocco=marocco)

        p1 = pynn.Population(100, pynn.IF_cond_exp)
        p2 = pynn.Population(100, pynn.IF_cond_exp)

        p = p1 + p2

        self.assertEqual(len(p), len(p1) + len(p2))

        pynn.run(1000)


@unittest.skipIf(not pymarocco_available(), "Test requires pymarocco")
class PopulationView(unittest.TestCase):

    def test_Constructor(self):
        import numpy
        import pymarocco

        marocco = pymarocco.PyMarocco()
        marocco.calib_backend = pymarocco.PyMarocco.CalibBackend.Default

        pynn.setup(marocco=marocco)

        N=10
        model = pynn.IF_cond_exp
        selector = numpy.array([ random.choice([True, False]) for x in range(0,N) ])
        pop = pynn.Population(N, model)
        pv = pynn.PopulationView(pop,selector)

        self.assertEqual(len(pv), len(numpy.where(selector==True)[0]))

        # now a selection with wrong size is given
        wrong_selector = numpy.array([ random.choice([True, False]) for x in range(0,2*N) ])
        with self.assertRaises(RuntimeError):
            pv = pynn.PopulationView(pop,wrong_selector)

        pynn.run(100)

    def test_Tset(self):
        pynn.setup()

        N = 10
        model = pynn.SpikeSourceArray
        pop = pynn.Population(N, model)

        all_spike_times = []
        for i in xrange(N):
            t = float(i)
            all_spike_times.append([t, t+1, t+2, t+3])
        pop.tset('spike_times', all_spike_times)

        import numpy
        all_spike_times_partial_numpy = []
        for spike_times in all_spike_times:
            all_spike_times_partial_numpy.append(spike_times)
        pop.tset('spike_times', all_spike_times_partial_numpy)

        all_spike_times_numpy = numpy.array(all_spike_times)
        pop.tset('spike_times', all_spike_times_numpy)


class Connector(unittest.TestCase):

    def test_OneToOneConnector(self):
        pynn.setup()
        conn = pynn.OneToOneConnector(weights=0.01, delays=1.0)

@unittest.skipIf(not pymarocco_available(), "Test requires pymarocco")
class Projection(unittest.TestCase):

    def test_FromListConnector(self):
        import numpy
        import pymarocco

        marocco = pymarocco.PyMarocco()
        marocco.calib_backend = pymarocco.PyMarocco.CalibBackend.Default

        pynn.setup(marocco=marocco)

        N=10
        model = pynn.IF_cond_exp
        pop = pynn.Population(N, model)

        connlist = [(0,1,0.1,0.1), (0,2,0.1,0.1), (0,3,0.1,0.1)]
        connector_e = pynn.FromListConnector(connlist)
        proj_e = pynn.Projection(pop,pop,connector_e, target='excitatory')

        pynn.run(100)


class Regressions(unittest.TestCase):

    def test_CellTypeInitialization(self):
        ts = [ 'EIF_cond_alpha_isfa_ista', 'EIF_cond_exp_isfa_ista',
                'HH_cond_exp', 'IF_brainscales_hardware', 'IF_cond_alpha',
                'IF_cond_exp', 'IF_cond_exp_gsfa_grr', 'IF_curr_alpha',
                'IF_curr_exp', 'IF_facets_hardware1',
                'SpikeSourceArray', 'SpikeSourceInhGamma', 'SpikeSourcePoisson']

        for cell in ts:
            self.assertIsNotNone(getattr(pynn, cell))


# currently commented out as it triggers a C++ assert, rendering the xml file empty
###    def test_FromListConnector_badlists(self):
###        import numpy
###        pynn.setup()
###
###        N=10
###        model = pynn.IF_brainscales_hardware
###        pop = pynn.Population(N, model)
###
###        # should triple-connection throw an error or be ignored?
###        # but no assertion
###        with self.assertRaises(RuntimeError):
###            connlist = [(0,1,0.1,0.1), (0,1,0.1,0.1), (0,1,0.1,0.1)]
###            connector_e = pynn.FromListConnector(connlist)
###            proj_e = pynn.Projection(pop,pop,connector_e, target='excitatory')


if __name__ == '__main__':
    unittest.main()
