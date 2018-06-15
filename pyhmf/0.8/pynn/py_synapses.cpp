#include "py_synapses.h"
#include "euter/synapses.h"
#include "pycellparameters/pyparameteraccess.h"

#include <boost/make_shared.hpp>
#include <boost/python/stl_iterator.hpp>

#include <string>

#define IMPLEMENT_WRAPPER(EUTER_CLASS)                                          \
Py##EUTER_CLASS* Py##EUTER_CLASS::_raw_constructor(bp::tuple, bp::dict kwargs)  \
{                                                                               \
	std::unique_ptr<Py##EUTER_CLASS> self(new Py##EUTER_CLASS(kwargs));         \
	return self.release();                                                      \
}                                                                               \
                                                                                \
Py##EUTER_CLASS::Py##EUTER_CLASS(bp::dict params)                               \
{                                                                               \
	boost::shared_ptr<EUTER_CLASS> impl = boost::make_shared<EUTER_CLASS>();    \
	ParameterProxy proxy = getParameterProxy(impl->mParams);                    \
	typedef bp::stl_input_iterator<bp::tuple> dict_iterator;                    \
	for(dict_iterator i(params.iteritems()), end; i!=end; ++i)                  \
	{                                                                           \
		bp::tuple t = (*i);                                                     \
		proxy.set(bp::extract<std::string>(t[0]), t[1]);                        \
	}                                                                           \
	_impl = impl;                                                               \
}

IMPLEMENT_WRAPPER(TsodyksMarkramMechanism)
IMPLEMENT_WRAPPER(AdditiveWeightDependence)
IMPLEMENT_WRAPPER(MultiplicativeWeightDependence)
IMPLEMENT_WRAPPER(AdditivePotentiationMultiplicativeDepression)
IMPLEMENT_WRAPPER(GutigWeightDependence)
IMPLEMENT_WRAPPER(SpikePairRule)

PySTDPMechanism::PySTDPMechanism(
		PySTDPTimingDependence& timing_dependence,
		double dendritic_delay_fraction
		): _impl(boost::make_shared<STDPMechanism>(
				timing_dependence.impl(),
				dendritic_delay_fraction
		)) {}

PySTDPMechanism::PySTDPMechanism(
		PySTDPWeightDependence& weight_dependence,
		double dendritic_delay_fraction
		): _impl(boost::make_shared<STDPMechanism>(
				weight_dependence.impl(), 
				dendritic_delay_fraction
		)) {}

PySTDPMechanism::PySTDPMechanism(
		PySTDPTimingDependence& timing_dependence,
		PySTDPWeightDependence& weight_dependence,
		double dendritic_delay_fraction
		): _impl(boost::make_shared<STDPMechanism>(
				timing_dependence.impl(),
				weight_dependence.impl(), 
				dendritic_delay_fraction
		)) {}


PySynapseDynamics::PySynapseDynamics(
		PyShortTermPlasticityMechanism fast,
		PySTDPMechanism slow
		) : _impl(boost::make_shared<SynapseDynamics>(fast.impl(), slow.impl()))
{
}

PySynapseDynamics::PySynapseDynamics(
		PyShortTermPlasticityMechanism fast
		) : _impl(boost::make_shared<SynapseDynamics>(fast.impl()))
{
}

PySynapseDynamics::PySynapseDynamics(
		PySTDPMechanism slow
		) : _impl(boost::make_shared<SynapseDynamics>(slow.impl()))
{
}

boost::shared_ptr<SynapseDynamics> PySynapseDynamics::impl()
{
	return _impl;
}
