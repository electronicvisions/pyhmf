"""
Stimulate a neuron with excitatory and inhibitory input. Test that
both stimuli have a measurable effect on the membrane.

Example:

    $ srun -p wafer --gres=cube1 python cube_spike_input_smoketest.py 4 89 --calib-path=/wang/data/calibration/wafer_4 --outdir=./ --neuron-number=12

Based on a script by AFK and SS

    Paul Mueller, <pmueller@kip.uni-heidelberg.de>

"""

import matplotlib
matplotlib.use("agg")

import pylogging
import pyhmf as pynn
from pymarocco import PyMarocco
from pymarocco import Routing
import pymarocco

import Coordinate as C

import pysthal
import numpy
import sys

import argparse


parser = argparse.ArgumentParser(description='smoke test for spike input')
parser.add_argument('wafer', type=int)
parser.add_argument('hicann', type=int)
parser.add_argument('--outdir', type=str)
parser.add_argument('--calib-path', type=str, default='./')
parser.add_argument('--neuron-number', type=int, default=3)
args = parser.parse_args()


###########
# LOGGING #
###########

pylogging.set_loglevel(pylogging.get("Default"), pylogging.LogLevel.INFO)
pylogging.set_loglevel(pylogging.get("marocco"), pylogging.LogLevel.DEBUG)
pylogging.set_loglevel(pylogging.get("control"), pylogging.LogLevel.DEBUG)
pylogging.set_loglevel(pylogging.get("sthal.HICANNConfigurator.Time"), pylogging.LogLevel.DEBUG)
pylogging.set_loglevel(pylogging.get("sthal.HICANNConfigurator"), pylogging.LogLevel.DEBUG)
pylogging.set_loglevel(pylogging.get("Calibtic"), pylogging.LogLevel.DEBUG)
pylogging.set_loglevel(pylogging.get("halbe"), pylogging.LogLevel.INFO)
pylogging.set_loglevel(pylogging.get("hicann-system"), pylogging.LogLevel.INFO)

####################
# MAROCCO SETTINGS #
####################

marocco = PyMarocco()
marocco.placement.setDefaultNeuronSize(4)
marocco.placement.use_output_buffer7_for_dnc_input_and_bg_hack = True
marocco.placement.minSPL1 = False
marocco.backend = PyMarocco.None   # .ESS, .Hardware
marocco.calib_backend = PyMarocco.XML
marocco.calib_path = args.calib_path
marocco.param_trafo.use_big_capacitors = False
marocco.roqt = "demo.roqt"
marocco.bio_graph = "demo.dot"
marocco.wafer_cfg = "wafer.dat"

# Set voltages in mV
marocco.param_trafo.alpha_v = 1.0
marocco.param_trafo.shift_v = 0.0

###################
# PYNN (==pyhmf)  #
###################

used_hicann = C.HICANNGlobal(
    C.HICANNOnWafer(C.Enum(args.hicann)),
    C.Wafer(args.wafer))

coord_analog =  C.AnalogOnHICANN(0)

pynn.setup(marocco=marocco)

params = {
                'cm'            :   0.2,
                'v_reset'       :  700,
                'v_rest'        :  500,
                'v_thresh'      :  1850,
                'e_rev_I'       : 0,
                'e_rev_E'       : 1800,
                'tau_syn_E'       : 10,
                'tau_syn_I'       : 10,
                'tau_m'         : 0.0001,
                'tau_refrac'    : 10.,
}

#####################################################################

# experiment duration in biological milliseconds
exc_region = [5000., 10000.]
inh_region = [15000., 20000.]
silent_region1 = [0., 5000.]
silent_region2 = [25000., 30000.]

start_offset = 500.

duration_ms = 30000.0

freq = .05

spikes_e = pynn.Population(1, pynn.SpikeSourceArray,
                           {'spike_times':numpy.arange(exc_region[0], exc_region[1], 15.)})
spikes_i = pynn.Population(1, pynn.SpikeSourceArray,
                           {'spike_times':numpy.arange(inh_region[0], inh_region[1], 15.)})
con_alltoall_e = pynn.AllToAllConnector(weights=100.)
con_alltoall_i = pynn.AllToAllConnector(weights=100.)

neuron_Number = args.neuron_number
pop_arr = []
for i in range(1, neuron_Number + 1):
    pop_arr.append(pynn.Population(1, pynn.IF_cond_exp, params))
    marocco.placement.add(pop_arr[i-1], used_hicann)
pop = pop_arr[neuron_Number - 1]

# Multiple projections for stronger input
pynn.Projection(spikes_e, pop, con_alltoall_e, target="excitatory")
pynn.Projection(spikes_e, pop, con_alltoall_e, target="excitatory")
pynn.Projection(spikes_e, pop, con_alltoall_e, target="excitatory")

pynn.Projection(spikes_i, pop, con_alltoall_i, target="inhibitory")
pynn.Projection(spikes_i, pop, con_alltoall_i, target="inhibitory")
pynn.Projection(spikes_i, pop, con_alltoall_i, target="inhibitory")

pynn.run(duration_ms)

pynn.end()


mapping_stats = marocco.getStats()

# load wafer config created by PyNN
w = pysthal.Wafer()
w.load(marocco.wafer_cfg)
h = w[used_hicann]

def get_denmems_for_pynn_nrn_with_pop(pop, nrn_index_in_pop):
    """
    Return used denmems of pop[nrn_index_in_pop]
    """
    bio_id = pymarocco.bio_id()

    if type(pop) is pynn.Population:
        bio_id.pop = pop.euter_id()
        # population-relative neuron idx
        bio_id.neuron = nrn_index_in_pop
    elif type(pop) is pynn.PopulationView:
        raise NotImplementedError("contact ECM on implementation details")

    return mapping_stats.getDenmems(bio_id)


denmems = get_denmems_for_pynn_nrn_with_pop(pop, 0)

# switch analog output of neuron to analog recorder
# using first denmem by default
h.enable_aout(denmems[0], coord_analog)

import pyhalbe
for denmem in denmems:
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_syntcx, 220)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_syntci, 210)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_t, 950)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_gl, 50)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_pl, 800)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.E_synx, 950)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_convi, 400)
    h.floating_gates.setNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_convx, 800)

fg = h.floating_gates
for ii in range(fg.getNoProgrammingPasses()):
    cfg = fg.getFGConfig(C.Enum(ii))
    cfg.fg_biasn = 0
    cfg.fg_bias = 0
    fg.setFGConfig(C.Enum(ii), cfg)

print '#########################################################################'
for denmem in denmems:
    print "analog parameters for denmem", denmem
    print 'E_l:  ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.E_l)
    print 'V_syntcx:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_syntcx)
    print 'V_syntci:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_syntci)
    print 'I_gl (tau_m):   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_gl)
    print 'I_pl (tau_ref):   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_pl)
    print 'V_syni:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_syni)
    print 'V_synx:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_synx)
    print 'I_convi:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_convi)
    print 'I_convx:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.I_convx)
    print 'V_convoffi:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_convoffi)
    print 'V_convoffx:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_convoffx)
    print 'V_threshold:   ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.V_t)
    print 'E_syni:    ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.E_syni)
    print 'E_synx:    ', h.floating_gates.getNeuron(denmem, pyhalbe.HICANN.neuron_parameter.E_synx)

print '#########################################################################'
h.use_big_capacitors(True)
h.set_speed_up_gl(pysthal.SpeedUp.NORMAL)
h.set_speed_up_gladapt(pysthal.SpeedUp.SLOW)
h.set_speed_up_radapt(pysthal.SpeedUp.SLOW)


db = pysthal.MagicHardwareDatabase()
import os

w.connect(db)

adc = h.analogRecorder(coord_analog)
recording_time = duration_ms/1e3/1e4
adc.setRecordingTime(recording_time)

w.configure(pysthal.HICANNv4Configurator())

runner = pysthal.ExperimentRunner(recording_time)

adc.activateTrigger()

w.start(runner)

v = adc.trace()
t = adc.getTimestamps()

regions = {
    "exc": exc_region,
    "inh": inh_region,
    "silent1": silent_region1,
    "silent2": silent_region2}

import pylab as p
if args.outdir:
    p.plot(t, v)
    p.xlabel("time [s]")
    p.ylabel("V_mem [converted mV]")

    for _, region in regions.items():
        p.axvspan((region[0] + start_offset) / 1000. / 10000., region[1] / 1000. / 10000.,
                  alpha=.1, color='k')

    p.savefig(os.path.join(
        args.outdir,
        "v.png"))

results = {}
for name, region in regions.items():

    mask = p.logical_and(
        t >= (region[0] + start_offset) / 1000. / 10000.,
        t <= region[1] / 1000. / 10000.)

    vv = v[mask]
    mu = p.mean(vv)
    std = p.std(vv, ddof=1)

    results[name] = (name, region[0] + start_offset, region[1], mu, std, len(vv))

f = open(os.path.join(
    args.outdir,
    "result.txt"), "w")

for name, l in results.items():
    f.write("\t".join([name] + [str(i) for i in l]) + "\n")

f.close()


# Calculate metrics and write output junit xml
def sigmadist(res1, res2):
    """
    Returns (mu_1 - mu_2) / sqrt(sigma_1^2 / n_1 + sigma_2^2 / n_2)
    """
    return (res1[3] - res2[3]) / p.sqrt(
        (res1[4] ** 2 / res1[5] + res2[4] ** 2 / res2[5]))

q_silent = sigmadist(results["silent1"], results["silent2"])
q_exc = sigmadist(results["exc"], results["silent1"])
q_inh = sigmadist(results["inh"], results["silent1"])


THRESH_SILENT = 10.
THRESH_EXC = 200.
THRESH_INH = -200.


from junit_xml import TestSuite, TestCase
t_exc = TestCase('Membrane potential with excitatory stimulus has higher membrane potential than without stimulus.')
if q_exc < THRESH_EXC:
    t_exc.add_error_info("Relative difference of mean membrane potential {} does not exceed threshold {}. Mean(exc) = {}, Mean(silent) = {}".format(
        q_exc, THRESH_EXC, results["exc"][3], results["silent1"][3]))

t_inh = TestCase('Membrane potential with inhibitory stimulus has lower membrane potential than without stimulus.')
if q_inh > THRESH_INH:
    t_inh.add_error_info("Relative difference of mean membrane potential {} does not exceed threshold {}. Mean(inh) = {}, Mean(silent) = {}".format(
        q_inh, THRESH_INH, results["inh"][3], results["silent1"][3]))

ts = TestSuite("Hicann 4 spike input smoke test", [t_exc, t_inh])

if args.outdir:
    f = open(os.path.join(args.outdir, "result.xml"), "w")
    f.write(TestSuite.to_xml_string([ts]))
