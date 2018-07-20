#!/usr/bin/env python
import unittest
import pyhmf as pynn

class GetSpikeCountsTest(unittest.TestCase):

    def test(self, max_neurons=10000):
        for n in range(1,max_neurons):
            self.provoke_crash(n)

    def provoke_crash(self, n_neurons):
        pop = pynn.Population(n_neurons, pynn.IF_cond_exp)
        selected = [n_neurons-1]
        pview = pop[selected]
        pview.get_spike_counts()

if __name__ == "__main__":
    unittest.main()
