#!/usr/bin/env python
# -*- coding: utf8 -*-
import numpy as np
import pyhmf as pynn
import pysthal
import unittest

def pymarocco_available():
    try:
        import pymarocco
    except ImportError:
        return False
    return True

def ESS_available():
    try:
        import pymarocco
        marocco = pymarocco.PyMarocco()
        marocco.backend = pymarocco.PyMarocco.ESS
        marocco.continue_despite_synapse_loss = True
        marocco.calib_backend = pymarocco.PyMarocco.CalibBackend.Default
        marocco.defects.backend = pymarocco.Defects.Backend.Without
        marocco.hicann_configurator = pysthal.HICANNConfigurator()

        pynn.setup(marocco=marocco)
        pynn.run(1.)
        return True
    except RuntimeError as err:
        if str(err) == "ESS not available (compile with ESS)":
            return False
        else:
            raise err

@unittest.skipIf(not pymarocco_available() or not ESS_available(), "Test requires pymarocco and ESS")
class TestSpikeRecording(unittest.TestCase):
    def test_cell_ids(self):
        """
        tests that [Population,PopulationView,Assembly].getSpikes() uses the
        expected cell ids, cf. issue #1955
        """

        import pymarocco

        marocco = pymarocco.PyMarocco()
        marocco.backend = pymarocco.PyMarocco.ESS
        marocco.experiment_time_offset = 5.e-7
        marocco.continue_despite_synapse_loss = True
        marocco.calib_backend = pymarocco.PyMarocco.CalibBackend.Default
        marocco.defects.backend = pymarocco.Defects.Backend.Without
        marocco.hicann_configurator = pysthal.HICANNConfigurator()

        setup_params = dict()
        if  pynn.__name__ == "pyhmf":
            setup_params['marocco']=marocco

        pynn.setup(**setup_params)

        # dummy target population
        p_dummy = pynn.Population(10, pynn.IF_cond_exp)

        # 1st Input Population
        p1 = pynn.Population(10, pynn.SpikeSourceArray)
        input_spikes1 = np.arange(1,11.,1.).reshape(10,1)
        for n,spikes in enumerate(input_spikes1):
            p1[n:n+1].set('spike_times', spikes.tolist())

        # 2nd Input Population
        p2 = pynn.Population(10, pynn.SpikeSourceArray)
        input_spikes2 = np.arange(11.,21.,1.).reshape(10,1)
        for n,spikes in enumerate(input_spikes2):
            p2[n:n+1].set('spike_times', spikes.tolist())

        p1.record()
        p2.record()

        # dummy connections otherwise input populations are not mapped.
        pynn.Projection(p1,p_dummy,pynn.OneToOneConnector())
        pynn.Projection(p2,p_dummy,pynn.OneToOneConnector())

        pynn.run(25.)

        # check that cell ids refer to the index in the Population.
        s_p1 = p1.getSpikes()
        s_p1 = s_p1[np.argsort(s_p1[:,1])] # sort by time
        self.assertTrue( np.array_equal(list(range(10)), s_p1[:,0]) )

        s_p2 = p2.getSpikes()
        s_p2 = s_p2[np.argsort(s_p2[:,1])] # sort by time
        self.assertTrue( np.array_equal(list(range(10)), s_p2[:,0]) )

        # for PopulationViews we also expect the index in the parent Population
        self.assertEqual( set(p2[0:1].getSpikes()[:,0]), set(range(1)) )
        self.assertEqual( set(p2[1:3].getSpikes()[:,0]), set(range(1,3)) )


        # In Assemblies, the cell id is equal to an offset given by the sum of
        # the Population sizes of the previous items (Population or
        # PopulationView), plus the index within in the Population.
        a = pynn.Assembly(p2[0:5],p1)
        s_a = a.getSpikes()
        # when sorted, ids should be: range(10,20) + range(5)
        s_a = s_a[np.argsort(s_a[:,1])] # sort by time
        self.assertTrue( np.array_equal(list(range(10,20))+list(range(5)), s_a[:,0]) )

if __name__ == '__main__':
    unittest.main()
