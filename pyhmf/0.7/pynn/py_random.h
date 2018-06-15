#pragma once

#include <string>
#include <vector>

#include "shared_ptr_fwd.h"

#include "pyhmf/boost_python_fwd.h"
#include "euter/random_traits.h"

class RNG;
class NativeRandom;
class RandomGenerator;
class RandomDistribution;

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
	virtual boost::shared_ptr<RandomDistribution> _getRandomDistribution(
        std::string distribution,
        bp::list parameters) const;

	virtual boost::shared_ptr<RandomGenerator> _getRNG() const;
};

class NativeRandomGenerator;
class PyNativeRNG : public PyAbstractRNG
{
public:
	PyNativeRNG();
	PyNativeRNG(random_int_t seed);

	virtual bp::object next(size_t n = 1,
			std::string distribution = "uniform",
			bp::list parameters = SentinelKeeper::emptyPyList,
			size_t mask_local = 0);

    bp::list permutation(bp::list const& lin);

	distribution_int_t randint(distribution_int_t a, distribution_int_t b);
	distribution_int_t randint(distribution_int_t b);
	distribution_float_t uniform(distribution_float_t a, distribution_float_t b);
	distribution_float_t normal(distribution_float_t mu, distribution_float_t std);
	distribution_int_t binomial(distribution_int_t n, distribution_float_t p);
	distribution_int_t poisson(distribution_float_t mean);
	distribution_float_t exponential(distribution_float_t lambda);

	virtual boost::shared_ptr<RandomDistribution> _getRandomDistribution(
        std::string distribution,
        bp::list parameters) const;

	virtual boost::shared_ptr<RandomGenerator> _getRNG() const;

	static PyNativeRNG defaultRNG;

private:
	boost::shared_ptr<NativeRandomGenerator> _impl;
};

class PyNumpyRNG : public PyAbstractRNG
{
public:
	PyNumpyRNG();
	PyNumpyRNG(random_int_t seed);
};

class PyGSLRNG : public PyAbstractRNG
{
public:
	PyGSLRNG();
	PyGSLRNG(random_int_t seed);
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

	boost::shared_ptr<RandomDistribution> _getDist() const;
private:
	boost::shared_ptr<RandomDistribution> _impl;
};

