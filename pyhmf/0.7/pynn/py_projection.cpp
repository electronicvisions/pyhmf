#include "py_projection.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/make_shared.hpp>

#include "py_assembly.h"
#include "py_connector.h"
#include "py_population.h"
#include "py_random.h"
#include "pyublas.h"
#include "errors.h"

#include "pyhmf/boost_python.h"
#include "pyhmf/objectstore.h"
#include "euter/random.h"
#include "euter/exceptions.h"
#include "euter/projection.h"
#include "euter/exceptions.h"

#include <boost/numeric/ublas/matrix.hpp>

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
PyProjectionPtr PyProjection::create(
	const PyAssembly & pre,
	const PyAssembly & post,
	PyConnectorPtr c,
	std::string s,
	std::string t,
	bp::object synapse_dynamics,
	std::string /* label */,
	const PyAbstractRNG & rng)
{
	boost::shared_ptr<SynapseDynamics> tmp_synapse_dynamics;
	if(synapse_dynamics.ptr() == emptyPyObject.ptr())
	{
		tmp_synapse_dynamics = boost::make_shared<SynapseDynamics>();
	}
	else
	{
		assert(bp::extract<PySynapseDynamics>(synapse_dynamics).check());
		tmp_synapse_dynamics = static_cast<PySynapseDynamics>(
				bp::extract<PySynapseDynamics>(synapse_dynamics)
				).impl();
	}

	try {
		auto p = boost::make_shared<PyProjection>();
		p->_impl = Projection::create(getStore(), pre._get(), post._get(), c->_getImpl(), rng._getRNG(), s, t, tmp_synapse_dynamics);
		return p;
	} catch(InvalidDimensions exc) {
		throw PyInvalidDimensionsError(exc.what());
	}
}

PyAssembly PyProjection::getPre() const
{
	return PyAssembly(boost::make_shared<Assembly>(_impl->pre()));
}

PyAssembly PyProjection::getPost() const
{
	return PyAssembly(boost::make_shared<Assembly>(_impl->post()));
}

/// d can be a single number, in which case all delays are set to this
/// value, or a list/1D array of length equal to the number of connections
/// in the projection, or a 2D array with the same dimensions as the
/// connectivity matrix (as returned by `getDelays(format)`).
void PyProjection::setDelays(double d)
{
	ProjectionMatrix & data = _impl->getDelays();
	data.set(d);
}
void PyProjection::setDelays(py_vector_type d)
{
	ProjectionMatrix & data = _impl->getDelays();
	data.set(d.as_ublas());
}
void PyProjection::setDelays(py_matrix_type d)
{
	ProjectionMatrix & data = _impl->getDelays();
	data.set(d.as_ublas());
}

/// w can be a single number, in which case all weights are set to this
/// value, or a list/1D array of length equal to the number of connections
/// in the projection, or a 2D array with the same dimensions as the
/// connectivity matrix (as returned by `getWeights(format)`).
/// Weights should be in nA for current-based and µS for conductance-based
/// synapses.
void PyProjection::setWeights(double w)
{
	ProjectionMatrix & data = _impl->getWeights();
	data.set(w);
}
void PyProjection::setWeights(py_vector_type w)
{
	ProjectionMatrix & data = _impl->getWeights();
	data.set(w.as_ublas());
}
void PyProjection::setWeights(py_matrix_type w)
{
	ProjectionMatrix & data = _impl->getWeights();
	data.set(w.as_ublas());
}

namespace
{
bp::object getImpl(std::string format, ProjectionMatrix & matrix)
{
	if (format == "list")
	{
		py_vector_type out(matrix.elements());
		auto i_out = out.as_ublas().begin();
		auto i_in = matrix.get().data().begin();
		auto i_end = matrix.get().data().end();

		while(i_in != i_end)
		{
			if(!std::isnan(*i_in))
			{
				assert(i_out != out.as_ublas().end());
				*i_out = *i_in;
				++i_out;
			}
			++i_in;
		}

		if(i_out != out.as_ublas().end())
		{
			throw std::runtime_error("Invalid size");
		}
		return bp::object(out);
	}
	else if (format == "matrix" || format == "array")
	{
		return bp::object(py_matrix_type(matrix.get()));
	}
	throw std::runtime_error("Invalid Fromat Type");
}

}

/// Get synaptic delays for all connections in this Projection.
/// Possible formats are: a list of length equal to the number of connections
/// in the projection, a 2D delay array (with NaN for non-existent
/// connections).
bp::object PyProjection::getDelays(std::string format, bool /* gather */)
{
	ProjectionMatrix & matrix = _impl->getDelays();
	return getImpl(format, matrix);
}

/// Get synaptic weights for all connections in this Projection.
/// Possible formats are: a list of length equal to the number of connections
/// in the projection, a 2D weight array (with NaN for non-existent
/// connections). Note that for the array format, if there is more than
/// one connection between two cells, the summed weight will be given.
bp::object PyProjection::getWeights(std::string format, bool /* gather */)
{
	ProjectionMatrix & matrix = _impl->getWeights();
	return getImpl(format, matrix);
}


// TODO remove after a reasonable amount of functions is implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

PyProjectionPtr connect(
        const PyAssembly &        source,
        const PyAssembly &        target,
        bp::object                weight,
        bp::object                delay,
        Projection::synapse_type  synapse_type,
        double                    p,
        const PyAbstractRNG & rng)
{
	auto c = PyFixedProbabilityConnector::create(p, true, weight, delay);
	NOT_IMPLEMENTED();
//	return PyProjection::create(source, target, c, "", synapse_type, rng);
}

/// Return the `i`th connection within the Projection.
void PyProjection::operator[](size_t i)
{
	NOT_IMPLEMENTED();
}

size_t PyProjection::euter_id() const
{
	return _impl->id();
}

/// Returns a human-readable description of the projection.
/// The output may be customized by specifying a different template
/// togther with an associated template engine (see ``pyNN.descriptions``).
/// If template is None, then a dictionary containing the template context
/// will be returned.
// TODO std::string PyProjection::describe(template, engine)
//{
//	NOT_IMPLEMENTED();
//}

/// Get parameters of the dynamic synapses for all connections in this
/// Projection.
bp::object PyProjection::getSynapseDynamics(std::string parameter_name,
                              std::string format,
                              bool gather)
{
	NOT_IMPLEMENTED();
}

/// Print synaptic weights to file. In the array format, zeros are printed
/// for non-existent connections.
bp::object PyProjection::printDelays(path file, std::string format, bool gather)
{
	NOT_IMPLEMENTED();
}

/// Print synaptic weights to file. In the array format, zeros are printed
/// for non-existent connections.
bp::object PyProjection::printWeights(path file, std::string format, bool gather)
{
	NOT_IMPLEMENTED();
}

/// Set delays to random values taken from rand_distr.
void PyProjection::randomizeDelays(PyRandomDistribution rand_distr)
{
	NOT_IMPLEMENTED();
}

/// Set parameters of the synapse dynamics to values taken from rand_distr
void PyProjection::randomizeSynapseDynamics(std::string param, PyRandomDistribution rand_distr)
{
	NOT_IMPLEMENTED();
}

/// Set weights to random values taken from rand_distr.
void PyProjection::randomizeWeights(PyRandomDistribution rand_distr)
{
	NOT_IMPLEMENTED();
}

/// Save connections to file in a format suitable for reading in with a
/// FromFileConnector.
void PyProjection::saveConnections(bp::object file, bool gather, bool compatible_output)
{
	pyublas::numpy_vector<double> data(_impl->pre().size() * _impl->post().size() * 4);

    for(size_t i=0; i<_impl->pre().size(); i++)
    {
		for(size_t j=0; j<_impl->post().size(); j++)
        {
			size_t pre_offset = 4 * _impl->post().size() * i;
			size_t post_offset = 4 * j;
			data[pre_offset + post_offset]     = i;
			data[pre_offset + post_offset + 1] = j;
			data[pre_offset + post_offset + 2] = _impl->getRawWeights()(i, j);
			data[pre_offset + post_offset + 3] = _impl->getRawDelays()(i, j);
        }
    }

	const long shape[2] = {-1, 4};
	data.reshape(2, shape);

	bp::dict metadata;

	file.attr("write")(data, metadata);

	/*
	bp::object pynn_files = bp::import("pyNN.recording.files");
	bp::object PickleFile = pynn_files.attr("PickleFile");

	bp::object my_file = PickleFile("/tmp/foo", "w");
	my_file.attr("write")(data, metadata);
	*/
}

/// Set parameters of the dynamic synapses for all connections in this
/// projection.
void PyProjection::setSynapseDynamics(std::string param, bp::object value)
{
	NOT_IMPLEMENTED();
}

/// Return the total number of connections.
/// - only local connections, if gather is False,
/// - all connections, if gather is True (default)
size_t PyProjection::size(bool gather) const
{
	return _impl->size();
}

size_t PyProjection::size() const
{
	return size(true);
}

std::ostream & operator<<(std::ostream & out, const PyProjection & p )
{
	return out << "Projection ( " << p._impl->pre() << " -> " << p._impl->post() << ")";
}

std::ostream & operator<<(std::ostream & out, const PyProjectionPtr & p)
{
	return (out << *p);
}
