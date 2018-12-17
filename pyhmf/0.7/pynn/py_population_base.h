#pragma once

#include "celliterator.h"
#include "py_id.h"
#include "py_assembly.h"
#include "py_assembly_base.h"

class PyPopulationBase : public PyAssemblyBase
{
public:

	virtual ~PyPopulationBase();

	/// Iterator over cell ids on all nodes.
	// all(); // Manually to begin() and end()?

	/// Return either a single cell (ID object) from the PyPopulation, if index
	/// is an integer, or a subset of the cells (PyPopulationView object), if
	/// index is a slice or array.
	/// Note that __getitem__ is called when using [] access, e.g.
	/// p = PyPopulation(...)
	/// p[2] is equivalent to p.__getitem__(2).
	/// p[3:6] is equivalent to p.__getitem__(slice(3, 6))
	virtual PyID operator[](const size_t index) const = 0;
	virtual PyPopulationView operator[](const bp::list& indices) const = 0;
	virtual PyPopulationView operator[](const pyublas::numpy_vector<long>& indices) const = 0;
	virtual PyPopulationView operator[](const bp::slice& selection) const = 0;

	/// A PyPopulation/PyPopulationView can be added to another Population,
	/// PyPopulationView or PyAssembly, returning an PyAssembly.
	/// defined extern in assembly.h
	PyAssembly operator+(const PyPopulationBase & b);
	PyAssembly operator+(const PyAssembly & b);

	/// Iterator over cell ids on the local node.
	// __iter__();
	virtual CellIterator<PyPopulationBase> begin() = 0;
	virtual CellIterator<PyPopulationBase> end() = 0;

	/// Return the total number of cells in the population (all nodes).
	/// PyNN API does not specify this one but rather __len__.
	/// However, most backends provide "size" as a property.
	size_t size() const;

	/// Return the label of this population.
	/// PyNN API does not specify this one but it is implemented in common.py
	std::string const& label() const;

	/// Return a copy of the cell type of this Population
	bp::object const celltype() const;

	/// Determine whether `variable` can be recorded from this population.
	bool can_record(std::string variable);

	/// Get the values of a parameter for every local cell in the population.
	std::vector<bp::object> get(std::string parameter_name, bool gather = false);

	/// Given the ID(s) of cell(s) in the PyPopulation, return its (their) index
	/// (order in the PyPopulation), counting only cells on the local MPI node.
	// id_to_local_index(id); // TODO confusing??


	/// Determine whether the cell with the given ID exists on the local MPI node.
	bool is_local(PyID id) const;

	/// Return the neuron closest to the specified position.
	PyID nearest(const bp::object & position) const;

	/// Set initial membrane potentials for all the cells in the population to
	/// random values.
	void randomInit(PyRandomDistribution rand_distr);


	/// 'Random' set. Set the value of parametername to a value taken from
	/// rand_distr, which should be a RandomDistribution object.
	void rset(std::string parametername, PyRandomDistribution rand_distr);

	/// Randomly sample n cells from the PyPopulation, and return a PyPopulationView
	/// object.
	bp::object sample(size_t n, PyRandomDistribution rng /* = None */);
	// TODO add useful default value

	/// Set one or more parameters for every cell in the population. param
	/// can be a dict, in which case val should not be supplied, or a string
	/// giving the parameter name, in which case val is the parameter value.
	/// val can be a numeric value, or list of such (e.g. for setting spike
	/// times).
	/// e.g. p.set("tau_m",20.0).
	/// p.set({'tau_m':20,'v_rest':-65})
	void set(std::string param, bp::object val);
	void set(const ParameterDict & parameters);

	/// 'Topographic' set. Set the value of parametername to the values in
	/// value_array, which must have the same dimensions as the PyPopulation.
	void tset(std::string const& parameter_name, npyarray const& value_array);
	void tset(std::string const& parameter_name, bp::object const& value_array);

	boost::shared_ptr<euter::PopulationView> _impl;

	friend class PyPopulationView;

	size_t euter_id() const;

protected:
	PyPopulationBase(const boost::shared_ptr<euter::PopulationView> & impl);

	// default ctor needed in PyPopulationView
	PyPopulationBase();

	void apply(std::function<void(euter::PopulationView &)> f);
	void apply(std::function<void(euter::PopulationView const&)> f) const;

	friend std::ostream & operator<<(std::ostream & out, const PyPopulationBase & p);
};
