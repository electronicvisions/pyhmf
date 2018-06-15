import pyhmf as pynn

class GetSpikeCountsTest():

    def __init__(self, max_neurons):

        for n in range(1,max_neurons):
            self.provoke_crash(n)

    def provoke_crash(self, n_neurons):
        print "{} neuron(s) in population".format(n_neurons)
        pop = pynn.Population(n_neurons, pynn.IF_cond_exp)
        selected = [n_neurons-1]
        pview = pop[selected]
        print "getting spike counts for neuron(s) {}".format(selected)
        pview.get_spike_counts()

if __name__ == "__main__":

    g = GetSpikeCountsTest(10000)
