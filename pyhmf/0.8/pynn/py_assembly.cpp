#include "py_assembly.h"

#include <algorithm>
#include <memory>
#include <iterator>

#include <boost/make_shared.hpp>

#include "celliterator.h"
#include "errors.h"
#include "py_id.h"
#include "py_population.h"
#include "euter/assembly.h"
#include "euter/exceptions.h"
#include "pyhmf/boost_python.h"


Assembly & PyAssembly::_get()
{
	return *_impl;
}

const Assembly & PyAssembly::_get() const
{
	return *_impl;
}

void PyAssembly::apply(std::function<void(PopulationView &)> f)
{
	for (auto& view : *_impl)
		f(view);
}

void PyAssembly::apply(std::function<void(PopulationView const&)> f) const
{
	for (auto& view : *_impl)
		f(view);
}

/// Return the total number of cells in the population (all nodes).
size_t PyAssembly::size() const
{
	return _impl->size();
}

std::string const& PyAssembly::label() const
{
	return _impl->label();
}

std::ostream & operator<<(std::ostream & out, const PyAssembly & p)
{
	return out << *(p._impl);
}

namespace { // helper

/// @brief Analyze positional arguments for PyAssembly::_raw_constructor.
///
/// The construction paramters could be 1-or-more positional or one single
/// sequence-type parameter
static std::vector<PyPopulationView> analyze_args(bp::tuple args) {
	size_t args_size = bp::len(args);
	if (args_size == 1 && bp::len(args[0]) >= 1) {
		// check for sequence in only positional parameter
		bp::extract<std::vector<PyPopulationView>> get_via_rvalue(args[0]);
		if (get_via_rvalue.check())
			return get_via_rvalue();
	}

	// fall back to try positional parameters
	std::vector<PyPopulationView> ret;
	for (size_t ii = 0; ii < args_size; ++ii) {
		ret.push_back(bp::extract<PyPopulationView>(args[ii]));
	}
	return ret;
}
} // anonymous namespace

PyAssembly* PyAssembly::_raw_constructor(bp::tuple args, bp::dict kwargs)
{
	std::vector<PyPopulationView> populationViews = analyze_args(args);

	std::string label = bp::extract<std::string>(kwargs.get("label", ""));
	std::unique_ptr<PyAssembly> assembly(new PyAssembly(populationViews, label));
	return assembly.release();
}

PyAssembly::PyAssembly(const boost::shared_ptr<Assembly> & p) :
    _impl(p)
{
}

PyAssembly::PyAssembly(const PyPopulationBase & p, std::string label) :
    _impl(boost::make_shared<Assembly>(*(p._impl), label))
{
}

PyAssembly::PyAssembly(const std::vector<PyPopulationView> & populationViews,
	std::string label)
{
	std::vector<PopulationView> views;
	for (auto p : populationViews)
	{
		views.push_back(*(p._impl));
	}
	_impl.reset(new Assembly(views, label));
}

/// An PyAssembly may be added to a PyPopulation, PyPopulationView or PyAssembly
/// with the '+' operator, returning a new PyAssembly, e.g.:
/// a2 = a1 + p
PyAssembly PyAssembly::operator+(const PyAssembly & other) const
{
	Assembly::list_type views;
	std::back_insert_iterator<Assembly::list_type> back_it(views);
	std::copy(_impl->begin(), _impl->end(), back_it);
	std::copy(other._impl->begin(), other._impl->end(), back_it);

	return PyAssembly(boost::make_shared<Assembly>(views));
}

PyAssembly PyAssembly::operator+(const PyPopulationBase & other) const
{
	return *this + PyAssembly(other);
}

// TODO remove after a reasonable amount of functions is implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

/// A PyPopulation, PyPopulationView or PyAssembly may be added to an existing
/// PyAssembly using the '+=' operator, e.g.:
/// a += p
PyAssembly& PyAssembly::operator+=(const PyAssembly & other)
{
    _impl->append(*(other._impl));
	return *this;
}

PyAssembly& PyAssembly::operator+=(const PyPopulationBase & other)
{
	return *this += PyAssembly(other);
}

/// Where index is an integer, return an ID.
/// Where index is a slice, list or numpy array, return a new PyAssembly
/// consisting of appropriate populations and (possibly newly created)
/// population views.
PyID PyAssembly::operator[](const size_t index) const
{
	if(index >= size())
	{
		std::stringstream msg;
		msg << "Index " << index << " not in assembly of size " << size();
		throw PyIndexError(msg.str());
	}
	
	size_t accumulative_size = 0;
	for(auto i = _get().populations().begin(); i != _get().populations().end(); ++i)
	{
		accumulative_size += i->size();
		if(index < accumulative_size)
		{
			return PyPopulationView(boost::make_shared<PopulationView>(*i))[index - (accumulative_size - i->size())];
		}
	}
	throw PyIndexError("Assembly has no populations.");
}

PyAssembly PyAssembly::operator[](const bp::list& indices) const
{
	pyublas::numpy_vector<long> np_indices(bp::len(indices));

	for(int i=0; i<bp::len(indices); ++i)
	{
		long index = bp::extract<long>(indices[i]);
		np_indices.as_ublas()[i] = index;
	}

	return this->operator[](np_indices);
}

// Nananananannanaaaasty. Well. It works. And should not leak. Hope so. Roger and over.
PyAssembly PyAssembly::operator[](const pyublas::numpy_vector<long>& indices) const
{
	// Set up mapping of (PopulationView → mask)
	std::map<PopulationView*, boost::dynamic_bitset<> > tmp;

	for(auto i=indices.as_ublas().begin(); i<indices.as_ublas().end(); ++i)
	{
		// Range check.
		size_t index = *i;
		if(index >= size())
		{
			std::stringstream msg;
			msg << "Index " << index << " not in assembly of size " << size();
			throw PyIndexError(msg.str());
		}

		// Find PopulationView containing index by looping over all members.
		size_t accumulative_size = 0;
		for(auto pv = _get().populations().begin(); pv != _get().populations().end(); ++pv)
		{
			// Check if index is in that PopulationView
			accumulative_size += pv->size();
			if(index < accumulative_size)
			{
				// If mapping does not contain PopulationView yet, add it with an empty mask.
				if(tmp.find(const_cast<PopulationView*>(&(*pv))) == tmp.end())
				{
					tmp[const_cast<PopulationView*>(&(*pv))] = boost::dynamic_bitset<>(pv->size());
					tmp[const_cast<PopulationView*>(&(*pv))].reset();
				}

				// Set appropriate bit.
				tmp[const_cast<PopulationView*>(&(*pv))][index - pv->population().firstNeuronId()] = true;
				break;
			}
		}
	}
	
	// Construct resulting PopulationViews
	std::vector<PopulationView> views;
	for (auto i=tmp.begin(); i!=tmp.end(); ++i) {
		PopulationView view = i->first->copy_with_mask(i->second);
		views.push_back(view);
	}

	// Construct resulting Assembly from these PVs
	boost::shared_ptr<Assembly> assembly = boost::make_shared<Assembly>(views);
	return PyAssembly(assembly);
}

PyAssembly PyAssembly::operator[](const bp::slice& selection) const
{
	std::vector<size_t> indices;

	std::vector<size_t> pool(size());
	std::iota(pool.begin(), pool.end(), 0);

	auto bounds = selection.get_indicies<>(pool.begin(), pool.end());

	for(auto ii = bounds.start; ii <= bounds.stop; std::advance(ii, bounds.step))
	{
		indices.push_back(*ii);
	}

	pyublas::numpy_vector<long> np_indices(indices.size());

	for(size_t i = 0; i < indices.size(); ++i)
	{
		np_indices.as_ublas()[i] = indices[i];
	}

	return this->operator[](np_indices);
}

CellIterator<PyAssembly> PyAssembly::begin()
{
	return CellIterator<PyAssembly>(0, boost::make_shared<PyAssembly>(*this));
}

CellIterator<PyAssembly> PyAssembly::end()
{
	return CellIterator<PyAssembly>(size(), boost::make_shared<PyAssembly>(*this));
}

/// Return the PyPopulation/PyPopulationView from within the PyAssembly that has
/// the given label. If no such PyPopulation exists, raise KeyError.
bp::object PyAssembly::get_population(std::string label) const
{
	NOT_IMPLEMENTED();
}

bool PyAssembly::operator==(const PyAssembly& right) const
{
	return (*_impl == *(right._impl));
}
