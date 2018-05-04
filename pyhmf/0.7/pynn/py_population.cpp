#include "py_population.h"

#include <boost/make_shared.hpp>

#include "pyhmf/boost_python.h"
#include "pyhmf/objectstore.h"
#include "py_space.h"
#include "celliterator.h"
#include "errors.h"
#include "py_id.h"
#include "euter/space.h"
#include "euter/exceptions.h"
#include "euter/population.h"
#include "euter/population_view.h"
#include "pycellparameters/pyparameteraccess.h"

// TODO remove after a reasonable amount of functions is implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

PyPopulation::~PyPopulation()
{

}

PopulationView const& PyPopulation::_get() const
{
	return *_impl;
}

size_t PyPopulation::euter_id() const
{
	return _get().population().id();
}

boost::shared_ptr<PopulationView> PyPopulation::createPopulation(
	size_t size,
	const bp::object& celltype,
	boost::shared_ptr<Structure> const& structure,
	const std::string& label)
{
	if(!size) {
		throw std::runtime_error("number of neurons must be a positiv integer");
	}

	CellType t = resolveCellType(celltype);
	PopulationPtr p  = Population::create(getStore(), size, t, structure, label);
	auto ret = boost::make_shared<PopulationView>(p);

	if(!ret->population().parameters().supported()) {
		std::string err = "BrainScaleS does not support this celltype: ";
		err += getCellTypeName(t);
		throw std::runtime_error(err);
	}

	return ret;
}

namespace
{
	size_t getSize(const bp::tuple size)
	{
		if (bp::len(size) < 1 && bp::len(size) > 3)
		{
			throw std::runtime_error("Invalid dimension :P");
		}

		size_t noCells = 1;
		for(bp::ssize_t ii = 0; ii < bp::len(size); ++ii)
		{
			noCells *= bp::extract<size_t>(size[ii]);
		}
		return noCells;
	}

	boost::shared_ptr<Structure> getStructure(const bp::object& structure)
	{
		if(structure.ptr() != SentinelKeeper::emptyPyObject.ptr()) // Did we get a structure handed over?
		{
			PyStructure const& ps = bp::extract<PyStructure const&>(structure);
			return ps._impl;
		}
		else
		{
			return boost::make_shared<Line>();
		}
	}

	boost::shared_ptr<Structure> getStructure(const bp::tuple size)
	{
		// TODO: Determine structure using the tuple length
		switch(bp::len(size))
		{
			case 1:
				return boost::make_shared<Line>();
			case 2:
				return boost::make_shared<Grid2D>(bp::extract<double>(size[0])/bp::extract<double>(size[1]));
			case 3:
				return boost::make_shared<Grid3D>(bp::extract<double>(size[0])/bp::extract<double>(size[1]),
					bp::extract<double>(size[0])/bp::extract<double>(size[2]));
			default:
				return boost::make_shared<Line>();
		}
	}
}

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
PyPopulation::PyPopulation(size_t size,
                           const bp::object & celltype,
                           const ParameterDict & cellparams,
                           const bp::object & structure,
                           std::string label) :
    PyPopulationBase(createPopulation(size, celltype, getStructure(structure), label))
{
	this->set(cellparams);

        // FIXME
	//	// call back to python to get positions
	//	if (structure.ptr() != emptyPyObject.ptr()) {

	//		// thread-safe: enclose with global interpreter lock
	//		PyGILState_STATE gstate = PyGILState_Ensure();
	//		auto ret = structure().attr("generate_positions")(size);
	//		PyGILState_Release(gstate);

	//		bp::numeric::array::set_module_and_type("numpy", "ndarray");
	//		bp::numeric::array x = bp::extract<bp::numeric::array> (ret);

	//		// check lengths
	//		assert(   (bp::len(x) == 3)
	//		       && (bp::len(x[0]) == static_cast<int>(size))
	//		       && (bp::len(x[1]) == static_cast<int>(size))
	//		       && (bp::len(x[2]) == static_cast<int>(size)));

	//		for (int j = 0; j < static_cast<int>(size); j++) {
	//			mPositions[j] = {
	//			                 bp::extract<double>(x[0][j]),
	//			                 bp::extract<double>(x[1][j]),
	//			                 bp::extract<double>(x[2][j])
	//			                };
	//		}
	//	}
}

PyPopulation::PyPopulation(const bp::tuple size,
                           const bp::object & celltype,
                           const ParameterDict & cellparams,
                           std::string label) :
    PyPopulationBase(createPopulation(getSize(size), celltype, getStructure(size), label))
{
	
	this->set(cellparams);
}

/// Return either a single cell (ID object) from the PyPopulation, if index
/// is an integer, or a subset of the cells (PyPopulationView object), if
/// index is a slice or array.
/// Note that __getitem__ is called when using [] access, e.g.
/// p = PyPopulation(...)
/// p[2] is equivalent to p.__getitem__(2).
/// p[3:6] is equivalent to p.__getitem__(slice(3, 6))
PyID PyPopulation::operator[](const size_t index) const
{
	if(index >= size())
	{
		std::stringstream msg;
		msg << "Index " << index << " not in population of size " << size();
		throw PyIndexError(msg.str());
	}
	
	PyID id(_get().population().firstNeuronId() + index, boost::make_shared<PyPopulation>(*this));
	return id;
}

PyPopulationView PyPopulation::operator[](const bp::slice& selection) const
{
	PyPopulationView view(*this, selection);
	return view;
}

PyPopulationView PyPopulation::operator[](const bp::list& indices) const
{
	PyPopulationView view(*this, indices);
	return view;
}

PyPopulationView PyPopulation::operator[](const pyublas::numpy_vector<long>& indices) const
{
	PyPopulationView view(*this, indices);
	return view;
}

CellIterator<PyPopulationBase> PyPopulation::begin()
{
	return CellIterator<PyPopulationBase>(0, boost::make_shared<PyPopulation>(*this));
}

CellIterator<PyPopulationBase> PyPopulation::end()
{
	return CellIterator<PyPopulationBase>(size(), boost::make_shared<PyPopulation>(*this));
}

/////////////////////
// Free Functions

// Create n cells all of the same type.
PyPopulation create(bp::object cellclass,
                    const ParameterDict& cellparams,
                    size_t n)
{
	PyPopulation population(n, cellclass, cellparams, SentinelKeeper::emptyPyObject, std::string("ladida"));
	return population;
}


// Record spikes (to a file)
void record(PyPopulation source, std::string filename)
{
	NOT_IMPLEMENTED();
}


// Record membrane potential (to a file)
void record_v(PyPopulation source, std::string filename)
{
	NOT_IMPLEMENTED();
}


// Record spikes (to a file)
void record_gsyn(PyPopulation source, std::string filename)
{
	NOT_IMPLEMENTED();
}
