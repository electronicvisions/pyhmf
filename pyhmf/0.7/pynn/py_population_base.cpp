#include <boost/make_shared.hpp>
#include "py_population_base.h"

#include "pyhmf/boost_python.h"
#include "py_id.h"
#include "errors.h"
#include "py_population_view.h"
#include "py_population.h"
#include "euter/exceptions.h"
#include "euter/population_view.h"
#include "euter/random.h"
#include "pycellparameters/pyparameteraccess.h"

PyPopulationBase::PyPopulationBase()
{
}

PyPopulationBase::PyPopulationBase(const boost::shared_ptr<PopulationView> & impl) :
    _impl(impl)
{
}

PyPopulationBase::~PyPopulationBase()
{
}

std::ostream & operator<<(std::ostream & out, const PyPopulationBase & p)
{
	return out << *(p._impl);
}

/// Return the total number of cells in the population (all nodes).
size_t PyPopulationBase::size() const
{
	return _impl->size();
}

std::string const& PyPopulationBase::label() const
{
	return _impl->label();
}

bp::object const PyPopulationBase::celltype() const
{
	return resolveCellType(_impl->population().type());
}

void PyPopulationBase::apply(std::function<void(PopulationView &)> f)
{
	f(*_impl);
}

void PyPopulationBase::apply(std::function<void(PopulationView const&)> f) const
{
	f(*_impl);
}

/// Get the values of a parameter for every local cell in the population.
std::vector<bp::object> PyPopulationBase::get(std::string parameter_name, bool /* gather */)
{
	std::vector<bp::object> parameters;
	const std::vector<ParameterProxy> & proxy = getPyParameterVector(*_impl);
	parameters.resize(proxy.size());
	for(size_t ii = 0; ii < proxy.size(); ++ii)
	{
		proxy[ii].get(parameter_name, parameters[ii]);
	}
	return parameters;
}

/// Set one or more parameters for every cell in the population. param
/// can be a dict, in which case val should not be supplied, or a string
/// giving the parameter name, in which case val is the parameter value.
/// val can be a numeric value, or list of such (e.g. for setting spike
/// times).
/// e.g. p.set("tau_m",20.0).
/// p.set({'tau_m':20,'v_rest':-65})
void PyPopulationBase::set(std::string parameter_name, bp::object val)
{
	std::vector<ParameterProxy> proxy = getPyParameterVector(*_impl);
	for(size_t ii = 0; ii < proxy.size(); ++ii)
	{
		proxy[ii].set(parameter_name, val);
	}
}

void PyPopulationBase::set(const ParameterDict &parameters)
{
	if(parameters.empty())
	{
		return;
	}

	const std::vector<ParameterProxy> & proxy = getPyParameterVector(*_impl);
	for(size_t ii = 0; ii < proxy.size(); ++ii)
	{
		for (auto item : parameters)
		{
			proxy[ii].set(item.first, item.second);
		}
	}
}

/// 'Random' set. Set the value of parametername to a value taken from
/// rand_distr, which should be a RandomDistribution object.
void PyPopulationBase::rset(std::string parameter_name, PyRandomDistribution rand_distr)
{
	auto dist = rand_distr._getDist();

	const std::vector<ParameterProxy> & proxy = getPyParameterVector(*_impl);
	if(dist->type() == RandomDistribution::INT)
	{
		std::vector<distribution_int_t> values(proxy.size());
		dist->next(values);
		for(size_t ii = 0; ii < proxy.size(); ++ii)
		{
		proxy[ii].set(parameter_name, bp::object(values[ii]));
		}
	}
	else if (dist->type() == RandomDistribution::REAL)
	{
		std::vector<distribution_float_t> values(proxy.size());
		dist->next(values);
		for(size_t ii = 0; ii < proxy.size(); ++ii)
		{
		proxy[ii].set(parameter_name, bp::object(values[ii]));
		}
	}
}

/// A PyPopulation/PyPopulationView can be added to another Population,
/// PyPopulationView or PyAssembly, returning an PyAssembly.
/// defined extern in assembly.h
PyAssembly PyPopulationBase::operator+(const PyPopulationBase & b)
{
	PyAssembly a(*this);
	a += b;
	return a;
}

PyAssembly PyPopulationBase::operator+(const PyAssembly & b)
{
	PyAssembly a(*this);
	a += b;
	return a;
}

// TODO remove after a reasonable amount of functions is implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

/// Set initial membrane potentials for all the cells in the population to
/// random values.
void PyPopulationBase::randomInit(PyRandomDistribution rand_distr)
{
	NOT_IMPLEMENTED();
}

/// Iterator over cell ids on the local node.
// __iter__();

/// `Topographic' call. Call the method methodname() for every cell in the
/// population. The argument to the method depends on the coordinates of
/// the cell. objarr is an array with the same dimensions as the
/// PyPopulation.
/// e.g. p.tcall("memb_init", vinitArray) calls
/// p.cell[i][j].memb_init(vInitArray[i][j]) for all i,j.
// _tcall(methodname, objarr);

/// Iterator over cell ids on all nodes.
// all(); // Manually to begin() and end()?

/// Determine whether `variable` can be recorded from this population.
bool PyPopulationBase::can_record(std::string variable)
{
	NOT_IMPLEMENTED();
}


/// Returns a human-readable description of the population.
/// The output may be customized by specifying a different template
/// togther with an associated template engine (see ``pyNN.descriptions``).
/// If template is None, then a dictionary containing the template context
/// will be returned.
// std::string describe(std::string tmpDemplate = population_default.txt, engine = default); // TODO to python?


/// Given the ID(s) of cell(s) in the PyPopulation, return its (their) index
/// (order in the PyPopulation), counting only cells on the local MPI node.
// id_to_local_index(id); // TODO confusing??


/// Connect a current source to all cells in the PyPopulation.
// inject(current_source); // TODO

/// Determine whether the cell with the given ID exists on the local MPI node.
bool PyPopulationBase::is_local(PyID id) const
{
	return true;
}


/// Return the neuron closest to the specified position.
PyID PyPopulationBase::nearest(const bp::object & position) const
{
	NOT_IMPLEMENTED();
}


/// Randomly sample n cells from the PyPopulation, and return a PyPopulationView
/// object.
bp::object PyPopulationBase::sample(size_t n, PyRandomDistribution rng)
{
	NOT_IMPLEMENTED();
}


/// 'Topographic' set. Set the value of parametername to the values in
/// value_array, which must have the same dimensions as the PyPopulation.
void PyPopulationBase::tset(std::string const& parameter_name, npyarray const& value_array)
{
	size_t value_array_size = 0;
#if BOOST_VERSION < 105600
	bp::object shape = value_array.getshape();
	size_t const len_shape = bp::len(shape);
	switch (len_shape) {
		case 2:
			value_array_size = bp::extract<size_t>(shape[0]) * bp::extract<size_t>(shape[1]);
			break;
		case 1:
			value_array_size = bp::extract<size_t>(shape[0]);
			break;
		default:
			throw PyInvalidDimensionsError("Invalid shape of value_array");
	}
#else
	auto shape = value_array.get_shape();
	int len_shape = value_array.get_nd();
	switch (len_shape) {
		case 2:
			value_array_size = shape[0] * shape[1];
			break;
		case 1:
			value_array_size = shape[1];
			break;
		default:
			throw PyInvalidDimensionsError("Invalid shape of value_array");
	}
#endif


	if (value_array_size != size())
		throw PyInvalidDimensionsError(
		    "Number of elements in the given array must be equal to the population size");

#if BOOST_VERSION < 105600
	bp::object flat_value_array = value_array.getflat();
#else
	auto flat_value_array = value_array.reshape(bp::make_tuple(value_array_size));
#endif

	std::vector<ParameterProxy> proxy = getPyParameterVector(*_impl);
	for (size_t ii = 0; ii < proxy.size(); ++ii)
		proxy[ii].set(parameter_name, flat_value_array[ii]);
}

void PyPopulationBase::tset(std::string const& parameter_name, bp::object const& value_array)
{
	if (PySequence_Check(value_array.ptr()) != 1)
		throw PyInvalidDimensionsError("Need sequence-type parameter for value_array");

	size_t const value_array_size = bp::len(value_array);
	if (value_array_size != size())
		throw PyInvalidDimensionsError(
		    "Number of elements in the given array must be equal to the population size");

	std::vector<ParameterProxy> proxy = getPyParameterVector(*_impl);
	for (size_t ii = 0; ii < proxy.size(); ++ii)
		proxy[ii].set(parameter_name, value_array[ii]);
}

size_t PyPopulationBase::euter_id() const
{
	return _impl->population().id();
}
