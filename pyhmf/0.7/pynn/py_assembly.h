#pragma once

#include <boost/shared_ptr.hpp>

#include "py_assembly_base.h"

template<typename ParentType>
class CellIterator;
class PyPopulationBase;

namespace euter {
class Assembly;
class PopulationView;
}

class PyAssembly : public PyAssemblyBase
{
public:
	/// Create an PyAssembly of Populations and/or PopulationViews.
	/// kwargs may contain a keyword argument 'label'.
	static PyAssembly * _raw_constructor(bp::tuple populations, bp::dict kwargs);
	// TODO expose as raw_function?
	// see http://wiki.python.org/moin/boost.python/HowTo#A.22Raw.22_constructor
	// TODO check for memleak...

	/// Create an PyNN Assembly from one or more PopulationView(s)
	PyAssembly(const PyPopulationBase & p,
			   std::string label = "");
	PyAssembly(const std::vector<PyPopulationView> & p,
			   std::string label = "");

	/// An PyAssembly may be added to a PyPopulation, PyPopulationView or PyAssembly
	/// with the '+' operator, returning a new PyAssembly, e.g.:
	/// a2 = a1 + p
	PyAssembly operator+(const PyAssembly & other) const;
	PyAssembly operator+(const PyPopulationBase & other) const;

	/// Where index is an integer, return an ID.
	/// Where index is a slice, list or numpy array, return a new PyAssembly
	/// consisting of appropriate populations and (possibly newly created)
	/// population views.
	PyID operator[](const size_t index) const;
	PyAssembly operator[](const bp::list& indices) const;
	PyAssembly operator[](const pyublas::numpy_vector<long>& indices) const;
	PyAssembly operator[](const bp::slice& selection) const;

	CellIterator<PyAssembly> begin();
	CellIterator<PyAssembly> end();
	
	/// A PyPopulation, PyPopulationView or PyAssembly may be added to an existing
	/// PyAssembly using the '+=' operator, e.g.:
	/// a += p
	PyAssembly& operator+=(const PyAssembly & other);
	PyAssembly& operator+=(const PyPopulationBase & other);

	/// Return the total number of cells in the population (all nodes).
	size_t size() const;

	/// Return the label of this assembly.
	/// PyNN API does not specify this one but it is implemented in common.py
	std::string const& label() const;

	/// Return the PyPopulation/PyPopulationView from within the PyAssembly that has
	/// the given label. If no such PyPopulation exists, raise KeyError.
	bp::object get_population(std::string label) const;

	euter::Assembly & _get();
	const euter::Assembly & _get() const;

	boost::shared_ptr<euter::Assembly> _impl;

	bool operator==(const PyAssembly& right) const;

	PyAssembly(const boost::shared_ptr<euter::Assembly> & p);
private:
	virtual void apply(std::function<void(euter::PopulationView &)> f);
	virtual void apply(std::function<void(euter::PopulationView const&)> f) const;

	friend std::ostream & operator<<(std::ostream & out, const PyAssembly & p);
};

/// A PyPopulation/PyPopulationView can be added to another Population,
/// PyPopulationView or PyAssembly, returning an PyAssembly.
/// defined extern in assembly.h
PyAssembly operator+(const PyPopulationBase &, const PyPopulationBase & b);
PyAssembly operator+(const PyPopulationBase &, const PyAssembly & b);
