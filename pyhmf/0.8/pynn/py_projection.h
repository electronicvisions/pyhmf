#pragma once

#include <iosfwd>
#include <boost/filesystem/path.hpp>

#include "pyublas.h"
#include "shared_ptr_fwd.h"
#include "py_assembly.h"
#include "py_synapses.h"
#include "py_random.h"
#include "pyhmf/boost_python_fwd.h"

typedef boost::filesystem::path path;

class PyProjection : public SentinelKeeper
{
public:
	/// Return the `i`th connection within the Projection.
	void operator[](size_t i);

	/// presynaptic_neurons and postsynaptic_neurons - Population, PopulationView
	/// or Assembly objects.
	/// source - string specifying which attribute of the presynaptic cell
	/// signals action potentials. This is only needed for
	/// multicompartmental cells with branching axons or
	/// dendrodendriticsynapses. All standard cells have a single
	/// source, and this is the default.
	/// target - string specifying which synapse on the postsynaptic cell to
	/// connect to. For standard cells, this can be 'excitatory' or
	/// 'inhibitory'. For non-standard cells, it could be 'NMDA', etc.
	/// If target is not given, the default values of 'excitatory' is
	/// used.
	/// method - a Connector object, encapsulating the algorithm to use for
	/// connecting the neurons.
	/// synapse_dynamics - a `standardmodels.SynapseDynamics` object specifying
	/// which synaptic plasticity mechanisms to use.
	/// rng - specify an RNG object to be used by the Connector.
	static PyProjectionPtr create(
	        const PyAssembly & presynaptic_population,
	        const PyAssembly & postsynaptic_population,
	        PyConnectorPtr method,
	        std::string source = "",
	        std::string target = "excitatory",
	        bp::object synapse_dynamics = emptyPyObject,
	        std::string label = "",
	        const PyAbstractRNG & rng = PyNativeRNG::defaultRNG // TODO fix default argument?
	        );

	PyAssembly getPre() const;
	PyAssembly getPost() const;

	/// Returns a human-readable description of the projection.
	/// The output may be customized by specifying a different template
	/// togther with an associated template engine (see ``pyNN.descriptions``).
	/// If template is None, then a dictionary containing the template context
	/// will be returned.
	// TODO std::string describe(template = projection_default.txt, engine = default);

	/// Get synaptic delays for all connections in this Projection.
	/// Possible formats are: a list of length equal to the number of connections
	/// in the projection, a 2D delay array (with NaN for non-existent
	/// connections).
	bp::object getDelays(std::string format = "list", bool gather = true);

	/// Get parameters of the dynamic synapses for all connections in this
	/// Projection.
	bp::object getSynapseDynamics(std::string parameter_name,
	                              std::string format = "list",
	                              bool gather = true);

	/// Get synaptic weights for all connections in this Projection.
	/// Possible formats are: a list of length equal to the number of connections
	/// in the projection, a 2D weight array (with NaN for non-existent
	/// connections). Note that for the array format, if there is more than
	/// one connection between two cells, the summed weight will be given.
	bp::object getWeights(std::string format = "list", bool gather = true);

	/// Print synaptic weights to file. In the array format, zeros are printed
	/// for non-existent connections.
	bp::object printDelays(path file, std::string format = "list", bool gather = true);

	/// Print synaptic weights to file. In the array format, zeros are printed
	/// for non-existent connections.
	bp::object printWeights(path file, std::string format = "list", bool gather = true);

	/// Set delays to random values taken from rand_distr.
	void randomizeDelays(PyRandomDistribution rand_distr);

	/// Set parameters of the synapse dynamics to values taken from rand_distr
	void randomizeSynapseDynamics(std::string param, PyRandomDistribution rand_distr);

	/// Set weights to random values taken from rand_distr.
	void randomizeWeights(PyRandomDistribution rand_distr);

	/// Save connections to file in a format suitable for reading in with a
	/// FromFileConnector.
	void saveConnections(bp::object file, bool gather = true, bool compatible_output = true);

	/// d can be a single number, in which case all delays are set to this
	/// value, or a list/1D array of length equal to the number of connections
	/// in the projection, or a 2D array with the same dimensions as the
	/// connectivity matrix (as returned by `getDelays(format='array')`).
	void setDelays(double d);
	void setDelays(py_vector_type d);
	void setDelays(py_matrix_type d);

	/// Set parameters of the dynamic synapses for all connections in this
	/// projection.
	void setSynapseDynamics(std::string param, bp::object value);

	/// w can be a single number, in which case all weights are set to this
	/// value, or a list/1D array of length equal to the number of connections
	/// in the projection, or a 2D array with the same dimensions as the
	/// connectivity matrix (as returned by `getWeights(format='array')`).
	/// Weights should be in nA for current-based and µS for conductance-based
	/// synapses.
	void setWeights(double w);
	void setWeights(py_vector_type d);
	void setWeights(py_matrix_type d);

	/// Return the total number of connections.
	/// - only local connections, if gather is False,
	/// - all connections, if gather is True (default)
	size_t size() const;

	/// Return the total number of local connections.
	size_t size(bool gather = true) const;

	/// returns euter projection id
	size_t euter_id() const;

	/// Return a histogram of synaptic weights.
	/// If min and max are not given, the minimum and maximum weights are
	/// calculated automatically.
	// TODO weightHistogram(min = None, max = None, nbins = 10);

	ProjectionPtr _impl;

private:
	friend std::ostream & operator<<(std::ostream & out, const PyProjection & p );
};

// Connect a source of spikes to a synaptic target.
PyProjectionPtr connect(
        const PyAssembly & source,
        const PyAssembly & target,
        bp::object weight = SentinelKeeper::emptyPyObject,
        bp::object delay = SentinelKeeper::emptyPyObject,
        std::string synapse_type = "",
        double p = 1.0,
        const PyAbstractRNG & rng = PyNativeRNG::defaultRNG
);

std::ostream & operator<<(std::ostream & out, const PyProjectionPtr & p);
