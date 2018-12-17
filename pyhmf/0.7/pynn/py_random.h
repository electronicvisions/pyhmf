#pragma once

#include <string>
#include <vector>

#include "shared_ptr_fwd.h"

#include "pyhmf/boost_python_fwd.h"
#include "euter/random_traits.h"

namespace euter {
class RandomGenerator;
class RandomDistribution;
}

class PyAbstractRNG
{
public:
	typedef size_t mask_local_t;
	virtual ~PyAbstractRNG();

	virtual bp::object next(size_t n = 1,
			std::string distribution = "uniform",
			bp::list parameters = SentinelKeeper::emptyPyList,
			size_t mask_local = 0) = 0;

	// Cant be pure virtual because it is not exposed to python :(
	virtual boost::shared_ptr<euter::RandomDistribution> _getRandomDistribution(
        std::string distribution,
        bp::list parameters) const;

	virtual boost::shared_ptr<euter::RandomGenerator> _getRNG() const;
};

namespace euter {
class NativeRandomGenerator;
}

class PyNativeRNG : public PyAbstractRNG
{
public:
	PyNativeRNG();
	PyNativeRNG(euter::random_int_t seed);

	virtual bp::object next(size_t n = 1,
			std::string distribution = "uniform",
			bp::list parameters = SentinelKeeper::emptyPyList,
			size_t mask_local = 0);

    bp::list permutation(bp::list const& lin);

	euter::distribution_int_t randint(euter::distribution_int_t a, euter::distribution_int_t b);
	euter::distribution_int_t randint(euter::distribution_int_t b);
	euter::distribution_float_t uniform(euter::distribution_float_t a, euter::distribution_float_t b);
	euter::distribution_float_t normal(euter::distribution_float_t mu, euter::distribution_float_t std);
	euter::distribution_int_t binomial(euter::distribution_int_t n, euter::distribution_float_t p);
	euter::distribution_int_t poisson(euter::distribution_float_t mean);
	euter::distribution_float_t exponential(euter::distribution_float_t lambda);

	virtual boost::shared_ptr<euter::RandomDistribution> _getRandomDistribution(
        std::string distribution,
        bp::list parameters) const;

	virtual boost::shared_ptr<euter::RandomGenerator> _getRNG() const;

	static PyNativeRNG defaultRNG;

private:
	boost::shared_ptr<euter::NativeRandomGenerator> _impl;
};

class PyNumpyRNG : public PyAbstractRNG
{
public:
	PyNumpyRNG();
	PyNumpyRNG(euter::random_int_t seed);
};

class PyGSLRNG : public PyAbstractRNG
{
public:
	PyGSLRNG();
	PyGSLRNG(euter::random_int_t seed);
};

class PyRandomDistribution {
public:
	PyRandomDistribution(
		std::string distribution = "uniform",
		bp::list parameters = SentinelKeeper::emptyPyList,
		const PyAbstractRNG & rng = PyNativeRNG(),
		bp::object boundaries = SentinelKeeper::emptyPyObject,
		std::string constrain = "clip"
	);

	bp::object next(size_t n = 1);

	boost::shared_ptr<euter::RandomDistribution> _getDist() const;
private:
	boost::shared_ptr<euter::RandomDistribution> _impl;
};

