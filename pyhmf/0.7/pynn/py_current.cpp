#include "euter/population_view.h"
#include "euter/current.h"
#include "pyhmf/objectstore.h"
#include "py_id.h"
#include "py_current.h"
#include "py_assembly.h"
#include "py_population_base.h"

#include "euter/exceptions.h"

#include <map>
#include <boost/make_shared.hpp>

using namespace euter;

ublas::vector<double> extractVector(const bp::object obj)
{
	ublas::vector<double> vec;

	bp::extract<bp::list> list(obj);
	if(list.check())
	{
		vec.resize(bp::len(list));

		for(unsigned i = 0; i < bp::len(list); i++)
		{
			vec(i) = bp::extract<double>(list()[i]);
		}
	}

	bp::extract<pyublas::numpy_vector<double> > npy(obj);
	if(npy.check())
	{
		vec = npy().as_ublas();
	}

	return vec;
}

PyCurrentSource::PyCurrentSource(boost::shared_ptr<CurrentSource> impl) : _impl(impl)
{
}

void PyCurrentSource::inject_into(const PyAssembly& target)
{
	_impl->inject_into(target._impl);
}

void PyCurrentSource::inject_into(const PyID& cell)
{
	PopulationPtr pop = cell.parent()->_impl->population_ptr();
	boost::dynamic_bitset<> mask(pop->size());
	mask.reset();
	
	mask[cell.id() - pop->firstNeuronId()] = true;
	
	PopulationView view(pop, mask);
	boost::shared_ptr<Assembly> assembly = boost::make_shared<Assembly>(view);
	_impl->inject_into(assembly);
}

void PyCurrentSource::inject_into(const bp::list& cells)
{
	std::map<PopulationPtr, boost::dynamic_bitset<> > tmp;

	for(int i=0; i<bp::len(cells); ++i)
	{
		PyID id = bp::extract<PyID>(cells[i]);
		PopulationPtr pop = id.parent()->_impl->population_ptr();
		if(tmp.find(pop) == tmp.end())
		{
			tmp[pop] = boost::dynamic_bitset<>(pop->size());
			tmp[pop].reset();
		}
		// TODO: Check if within bounds
		tmp[pop][id.id() - pop->firstNeuronId()] = true;
	}

	std::vector<PopulationView> views;

	for (auto i=tmp.begin(); i!=tmp.end(); ++i) {
		PopulationView view(i->first, i->second);
		views.push_back(view);
	}

	boost::shared_ptr<Assembly> assembly = boost::make_shared<Assembly>(views);
	_impl->inject_into(assembly);
}

PyDCSource::PyDCSource(double amplitude, double start, double stop) :
	PyCurrentSource(DCSource::create(getStore(), amplitude, start, stop))
{
}


PyStepCurrentSource::PyStepCurrentSource(bp::object times, bp::object amplitudes) :
	PyCurrentSource(StepCurrentSource::create(getStore(), extractVector(times), extractVector(amplitudes)))
{
}


PyACSource::PyACSource(double amplitude,
					   double offset,
					   double frequency,
					   double phase,
					   double start,
					   double stop) :
	PyCurrentSource(ACSource::create(getStore(), amplitude, offset, frequency, phase, start, stop))
{
}

PyNoisyCurrentSource::PyNoisyCurrentSource(double mean,
										   double stdev,
										   double dt,
										   double start,
										   double stop/*, rng*/) :
	PyCurrentSource(NoisyCurrentSource::create(getStore(), mean, stdev, dt, start, stop))
{
}
