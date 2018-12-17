#pragma once

#include "pyhmf/boost_python_fwd.h"

#ifndef PYPLUSPLUS
#include "euter/synapses.h"
#endif

namespace euter {
class ShortTermPlasticityMechanism;
}

#define DEFINE_WRAPPER(EUTER_CLASS, EUTER_BASE)                                \
namespace euter {                                                              \
class EUTER_CLASS;                                                             \
}                                                                              \
struct Py##EUTER_CLASS : public Py##EUTER_BASE                                 \
{                                                                              \
	Py##EUTER_CLASS(bp::dict params);                                          \
	~Py##EUTER_CLASS() {}                                                      \
	static Py##EUTER_CLASS* _raw_constructor(bp::tuple, bp::dict kwargs);      \
};          

class PyShortTermPlasticityMechanism
{
public:
	virtual ~PyShortTermPlasticityMechanism() {}
	boost::shared_ptr<euter::ShortTermPlasticityMechanism> impl() { return _impl; }
protected:
	boost::shared_ptr<euter::ShortTermPlasticityMechanism> _impl;
};

DEFINE_WRAPPER(TsodyksMarkramMechanism, ShortTermPlasticityMechanism)

namespace euter {
class STDPWeightDependence;
}

class PySTDPWeightDependence
{
public:
	virtual ~PySTDPWeightDependence() {}
	boost::shared_ptr<euter::STDPWeightDependence> impl() { return _impl; }
protected:
	boost::shared_ptr<euter::STDPWeightDependence> _impl;
};

DEFINE_WRAPPER(AdditiveWeightDependence, STDPWeightDependence)
DEFINE_WRAPPER(MultiplicativeWeightDependence, STDPWeightDependence)
DEFINE_WRAPPER(AdditivePotentiationMultiplicativeDepression, STDPWeightDependence)
DEFINE_WRAPPER(GutigWeightDependence, STDPWeightDependence)

namespace euter {
class STDPTimingDependence;
}
class PySTDPTimingDependence
{
public:
	virtual ~PySTDPTimingDependence() {}
	boost::shared_ptr<euter::STDPTimingDependence> impl() { return _impl; }
protected:
	boost::shared_ptr<euter::STDPTimingDependence> _impl;
};

DEFINE_WRAPPER(SpikePairRule, STDPTimingDependence)

namespace euter {
class STDPMechanism;
}

class PySTDPMechanism
{
public:
	PySTDPMechanism(
			PySTDPTimingDependence& timing_dependence,
			double dendritic_delay_fraction = 1.0
			);
	PySTDPMechanism(
			PySTDPWeightDependence& weight_dependence,
			double dendritic_delay_fraction = 1.0
			);
	PySTDPMechanism(
			PySTDPTimingDependence& timing_dependence,
			PySTDPWeightDependence& weight_dependence,
			double dendritic_delay_fraction = 1.0
			);
	boost::shared_ptr<euter::STDPMechanism> impl() { return _impl; }
private:
	boost::shared_ptr<euter::STDPMechanism> _impl;
};

namespace euter {
class SynapseDynamics;
}

class PySynapseDynamics
{
public:
	PySynapseDynamics(
			PyShortTermPlasticityMechanism fast
			);
	PySynapseDynamics(
			PySTDPMechanism slow
			);
	PySynapseDynamics(
			PyShortTermPlasticityMechanism fast,
			PySTDPMechanism slow
			);
	boost::shared_ptr<euter::SynapseDynamics> impl();
private:
	boost::shared_ptr<euter::SynapseDynamics> _impl;
};
