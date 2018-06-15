#pragma once

#include "py_population_base.h"
#include "pyublas.h"

class PyPopulationView : public PyPopulationBase
{
public:
	/// Create a view of a subset of neurons within a parent PyPopulation or
	/// PyPopulationView.
	/// selector - a slice or numpy mask array. The mask array should either be
	/// a boolean array of the same size as the parent, or an
	/// integer array containing cell indices, i.e. if p.size == 5,
	/// PyPopulationView(p, array([False, False, True, False, True]))
	/// PyPopulationView(p, array([2,4]))
	/// PyPopulationView(p, slice(2,5,2))
	/// will all create the same view.
	PyPopulationView(PyPopulation const& parent);
	PyPopulationView(const PyPopulation & parent,
	                 std::string label);
	PyPopulationView(PyPopulation const& parent, pyublas::numpy_vector<bool> selector);
	PyPopulationView(const PyPopulation & parent,
	                 pyublas::numpy_vector<bool> selector,
	                 std::string label);
	PyPopulationView(PyPopulation const& parent, bp::list selector);
	PyPopulationView(const PyPopulation & parent,
	                 bp::list selector,
	                 std::string label);
	PyPopulationView(PyPopulation const& parent, pyublas::numpy_vector<long> selector);
	PyPopulationView(const PyPopulation & parent,
	                 pyublas::numpy_vector<long> selector,
	                 std::string label);
	PyPopulationView(PyPopulation const& parent, bp::slice selector);
	PyPopulationView(const PyPopulation & parent,
	                 bp::slice selector,
	                 std::string label);

	// default ctor needed in pywrap's from_pyiterable converter
	PyPopulationView();

	virtual ~PyPopulationView();

	pyublas::numpy_vector<long> mask() const;

	PyID operator[](const size_t index) const;
	PyPopulationView operator[](const bp::list& indices) const;
	PyPopulationView operator[](const pyublas::numpy_vector<long>& indices) const;
	PyPopulationView operator[](const bp::slice& selection) const;

	CellIterator<PyPopulationBase> begin();
	CellIterator<PyPopulationBase> end();

	PyPopulationView(const boost::shared_ptr<PopulationView>& impl);
};
