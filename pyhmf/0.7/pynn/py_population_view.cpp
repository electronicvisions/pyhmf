#include "py_population_view.h"

#include <limits>
#include <boost/assign/std/vector.hpp>
#include <boost/make_shared.hpp>

#include "indexiterator.h"
#include "errors.h"
#include "py_id.h"
#include "py_population.h"
#include "pyublas.h"
#include "pyhmf/boost_python.h"
#include "euter/exceptions.h"
#include "euter/population.h"
#include "euter/population_view.h"

PyPopulationView::~PyPopulationView()
{

}

boost::dynamic_bitset<> createMask(size_t size, bool v)
{
	boost::dynamic_bitset<> mask(size);
	if (v) {
		mask.flip();
	}
	return mask;
}

boost::dynamic_bitset<> createMask(size_t size, bp::list l)
{
	boost::dynamic_bitset<> mask(size);
	for (int ii = 0; ii < bp::len(l); ++ii) {
		size_t idx = bp::extract<size_t>(l[ii]);
		if (idx >= size) {
			throw std::runtime_error("Index out of range");
		}
		mask.set(idx);
	}
	return mask;
}

boost::dynamic_bitset<> createMask(size_t size, pyublas::numpy_vector<long> v)
{
	boost::dynamic_bitset<> mask(size);
	for (auto ii : v.as_ublas()) {
		if (ii > static_cast<long>(size)) {
			throw std::runtime_error("Index out of range");
		}
		mask.set(ii);
	}
	return mask;
}

boost::dynamic_bitset<> createMask(size_t size, pyublas::numpy_vector<bool> v)
{
	if (v.size() != size) {
		throw std::runtime_error("Size of mask does not match size of population");
	}

	boost::dynamic_bitset<> mask(size);
	for (size_t ii = 0; ii < size; ++ii) {
		mask[ii] = v(ii);
	}
	return mask;
}

boost::dynamic_bitset<> createMask(size_t size, bp::slice selector)
{
	boost::dynamic_bitset<> mask(size);

	bp::slice::range< index_iterator > indices;
	indices = selector.get_indicies<>(index_iterator(0),
	                                  index_iterator(size + 1));

	for (index_iterator ii = indices.start;
	     ii <= indices.stop && *ii < static_cast<ssize_t>(size) && *ii >= 0;
	     std::advance(ii, indices.step))
	{
		mask.set(*ii);
	}
	return mask;
}

template <typename T>
boost::shared_ptr<PopulationView> createView(PyPopulation const& p, T selector)
{
	auto view = boost::make_shared<PopulationView>(
	    p._get().copy_with_mask(createMask(p._get().mask().size(), selector)));
	return view;
}

template <typename T>
boost::shared_ptr<PopulationView> createView(const PyPopulation & p,
                                             T selector,
                                             std::string label)
{
	auto view = createView(p, selector);
	view->setLabel(label);
	return view;
}

PyPopulationView::PyPopulationView(PyPopulation const& parent)
    : PyPopulationBase(createView(parent, true))
{
}

PyPopulationView::PyPopulationView(const PyPopulation & parent, std::string label) :
	PyPopulationBase(createView(parent, true, label))
{
}

PyPopulationView::PyPopulationView(PyPopulation const& parent, bp::slice selector)
    : PyPopulationBase(createView(parent, selector))
{
}

PyPopulationView::PyPopulationView(const PyPopulation & parent,
                                   bp::slice selector,
                                   std::string label) :
    PyPopulationBase(createView(parent, selector, label))
{
}

PyPopulationView::PyPopulationView(PyPopulation const& parent, bp::list selector)
    : PyPopulationBase(createView(parent, selector))
{
}

PyPopulationView::PyPopulationView(const PyPopulation & parent,
                                   bp::list selector,
                                   std::string label) :
    PyPopulationBase(createView(parent, selector, label))
{
}

PyPopulationView::PyPopulationView(PyPopulation const& parent, pyublas::numpy_vector<long> selector)
    : PyPopulationBase(createView(parent, selector))
{
}

PyPopulationView::PyPopulationView(const PyPopulation & parent,
                                   pyublas::numpy_vector<long> selector,
                                   std::string label) :
    PyPopulationBase(createView(parent, selector, label))
{
}

PyPopulationView::PyPopulationView(PyPopulation const& parent, pyublas::numpy_vector<bool> selector)
    : PyPopulationBase(createView(parent, selector))
{
}

PyPopulationView::PyPopulationView(const PyPopulation & parent,
                                   pyublas::numpy_vector<bool> selector,
                                   std::string label) :
    PyPopulationBase(createView(parent, selector, label))
{
}

PyPopulationView::PyPopulationView(const boost::shared_ptr<PopulationView>& impl) : PyPopulationBase(impl)
{
}

PyPopulationView::PyPopulationView()
{
}

pyublas::numpy_vector<long> PyPopulationView::mask() const
{
	const boost::dynamic_bitset<> & mask = _impl->mask();
	pyublas::numpy_vector<long> result(_impl->size(), std::numeric_limits<int>::max());
	auto out = result.as_ublas().begin();
	for(size_t ii = 0; ii < mask.size(); ++ii)
	{
		if(mask[ii])
		{
			assert(out != result.as_ublas().end());
			*out = ii;
			++out;
		}
	}
	return result;
}

PyID PyPopulationView::operator[](const size_t index) const
{
	if(index >= size())
	{
		std::stringstream msg;
		msg << "Index " << index << " not in population view of size " << size();
		throw PyIndexError(msg.str());
	}
	PyID id(mask()[index] + _impl->population().firstNeuronId(), boost::make_shared<PyPopulationView>(*this));
	return id;
}

PyPopulationView PyPopulationView::operator[](const bp::slice& selection) const
{
	pyublas::numpy_vector<long> old_mask = mask();
	auto bounds = selection.get_indicies<>(old_mask.as_ublas().begin(), old_mask.as_ublas().end());

	boost::dynamic_bitset<> new_mask(_impl->mask());
	new_mask.reset();
	
	for(auto ii = bounds.start; ii <= bounds.stop; std::advance(ii, bounds.step))
	{
		if((size_t)*ii > new_mask.size())
			throw std::runtime_error("Index out of range");
		new_mask[*ii] = true;
	}

	PopulationView view = _impl->copy_with_mask(new_mask);
	return PyPopulationView(boost::make_shared<PopulationView>(view));
}

PyPopulationView PyPopulationView::operator[](const bp::list& indices) const
{
	pyublas::numpy_vector<long> old_mask = mask();

	boost::dynamic_bitset<> new_mask(_impl->mask());
	new_mask.reset();
	
	for(int i=0; i<bp::len(indices); ++i)
	{
		if((size_t)old_mask[bp::extract<size_t>(indices[i])] > new_mask.size())
			throw std::runtime_error("Index out of range");
		new_mask[old_mask[bp::extract<long>(indices[i])]] = true;
	}

	PopulationView view = _impl->copy_with_mask(new_mask);
	return PyPopulationView(boost::make_shared<PopulationView>(view));
}

PyPopulationView PyPopulationView::operator[](const pyublas::numpy_vector<long>& indices) const
{
	pyublas::numpy_vector<long> old_mask = mask();

	boost::dynamic_bitset<> new_mask(_impl->mask());
	new_mask.reset();
	
	for(auto i=indices.as_ublas().begin(); i<indices.as_ublas().end(); ++i)
	{
		if((size_t)old_mask[*i] > new_mask.size())
			throw std::runtime_error("Index out of range");
		new_mask[old_mask[*i]] = true;
	}

	PopulationView view = _impl->copy_with_mask(new_mask);
	return PyPopulationView(boost::make_shared<PopulationView>(view));
}

CellIterator<PyPopulationBase> PyPopulationView::begin()
{
	return CellIterator<PyPopulationBase>(0, boost::make_shared<PyPopulationView>(*this));
}

CellIterator<PyPopulationBase> PyPopulationView::end()
{
	return CellIterator<PyPopulationBase>(size(), boost::make_shared<PyPopulationView>(*this));
}
