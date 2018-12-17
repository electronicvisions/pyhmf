#pragma once

#include "py_population_view.h"
#include "py_models.h"
#include "py_space.h"

namespace euter {
class Population;
}

/// The PyNN 0.7 population API as C++ code ...
class PyPopulation : public PyPopulationBase
{
public:
	virtual ~PyPopulation();

	/// Create a population of neurons all of the same type.
	/// size - number of cells in the PyPopulation. For backwards-compatibility,
	/// n may also be a tuple giving the dimensions of a grid,
	/// e.g. n=(10,10) is equivalent to n=100 with structure=Grid2D()
	/// cellclass should either be a standardized cell class (a class inheriting
	/// from common.standardmodels.StandardCellType) or a string giving the
	/// name of the simulator-specific model that makes up the population.
	/// cellparams should be a dict which is passed to the neuron model
	/// constructor
	/// structure should be a Structure instance.
	/// label is an optional name for the population.
	PyPopulation(size_t size,
	             const bp::object & cellclass,
	             const ParameterDict & cellparams = ParameterDict(),
	             const bp::object & structure  = emptyPyObject,
	             std::string label = "");

	PyPopulation(const bp::tuple size,
	             const bp::object & cellclass,
	             const ParameterDict & cellparams = ParameterDict(),
	             std::string label = "");

	/// Return either a single cell (ID object) from the PyPopulation, if index
	/// is an integer, or a subset of the cells (PyPopulationView object), if
	/// index is a slice or array.
	/// Note that __getitem__ is called when using [] access, e.g.
	/// p = PyPopulation(...)
	/// p[2] is equivalent to p.__getitem__(2).
	/// p[3:6] is equivalent to p.__getitem__(slice(3, 6))
	PyID operator[](const size_t index) const;
	PyPopulationView operator[](const bp::list& indices) const;
	PyPopulationView operator[](const pyublas::numpy_vector<long>& indices) const;
	PyPopulationView operator[](const bp::slice& selection) const;

	CellIterator<PyPopulationBase> begin();
	CellIterator<PyPopulationBase> end();

	euter::PopulationView const& _get() const;

private:
	static boost::shared_ptr<euter::PopulationView> createPopulation(
	        size_t size,
	        const bp::object& cellclass,
	        boost::shared_ptr<euter::Structure> const& structure,
	        const std::string& label);
};

// Create n cells all of the same type.
PyPopulation create(bp::object cellclass,
                    const ParameterDict & cellparams = ParameterDict(),
                    size_t n = 1);

// Record spikes (to a file)
void record(PyPopulation source, std::string filename);

// Record membrane potential (to a file)
void record_v(PyPopulation source, std::string filename);

// Record spikes (to a file)
void record_gsyn(PyPopulation source, std::string filename);
