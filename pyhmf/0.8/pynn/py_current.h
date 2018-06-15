#pragma once

#include "pyhmf/boost_python_fwd.h"

class CurrentSource;

class PyPopulationBase;
class PyAssembly;

class PyCurrentSource
{
public:
	PyCurrentSource(boost::shared_ptr<CurrentSource> impl);
	virtual ~PyCurrentSource() {}
	void inject_into(const PyAssembly& target);
	void inject_into(const PyID& cell);
	void inject_into(const bp::list& cells);
protected:
	boost::shared_ptr<CurrentSource> _impl;
};

class PyDCSource : public PyCurrentSource
{
public:
	PyDCSource(double amplitude=1.0, double start=0.0, double stop=0);
};

class PyStepCurrentSource : public PyCurrentSource
{
public:
	PyStepCurrentSource(bp::object times, bp::object amplitudes);
};

class PyACSource : public PyCurrentSource
{
public:
	PyACSource(double amplitude=1.0,
			   double offset=0.0,
			   double frequency=10.0,
			   double phase=0.0,
			   double start=0.0,
			   double stop=0.0);
};

class PyNoisyCurrentSource : public PyCurrentSource
{
public:
	PyNoisyCurrentSource(double mean,
						 double stdev,
						 double dt=0.0,
						 double start=0.0,
						 double stop=0.0/*, rng*/);
};

