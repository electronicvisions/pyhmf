#include "py_random.h"
#include "pyublas.h"

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <boost/make_shared.hpp>

#include "pyhmf/boost_python.h"
#include "euter/exceptions.h"
#include "euter/nativerandomgenerator.h"
#include "euter/nativerandomdistributions.h"
#include "euter/random.h"

#include "ztl/pack/pack.h"
#include "ztl/pack/get.h"

PyAbstractRNG::~PyAbstractRNG() {}

boost::shared_ptr<RandomDistribution>
PyAbstractRNG::_getRandomDistribution(std::string, bp::list) const
{
	return boost::shared_ptr<RandomDistribution>();
}

boost::shared_ptr<RandomGenerator> PyAbstractRNG ::_getRNG() const
{
	return boost::shared_ptr<RandomGenerator>();
}

PyNativeRNG PyNativeRNG::defaultRNG = PyNativeRNG();

namespace
{

template <typename T>
struct object_type
{
	typedef bp::object type;
};

template<typename T>
T help_extract(bp::object obj)
{
	return bp::extract<T>(obj);
}

template <typename T, typename ... Args>
struct NRDProxyImpl;

// This and its derived NRDProxyImpl structs provide functions to resolve
// the requested random distribution to a euter NativeRandomDistribution
// class
// A NRDProxyImpl can be created using the Get helper struct
struct NRDProxy
{
	typedef boost::shared_ptr<NativeRandomGenerator> rng_type;
	typedef boost::shared_ptr<NativeRandomDistribution> dist_type;
	typedef typename std::unique_ptr<NRDProxy> proxy_type;
	typedef NativeRandomGenerator::rng_type raw_rng_type;

	virtual ~NRDProxy() {}
	// Create 1 random number, the return value is that value
	virtual bp::object operator()() = 0;
	// Create size random numbers, the return value is a numpy array
	virtual bp::object operator()(size_t size) = 0;
	// Return underlying NativeRandomDistribution
	virtual dist_type get_distribution() const = 0;

	template <typename T>
	struct Get
	{
		typedef typename T::template GetArgumentTypes<NRDProxyImpl, T>::type type;
	};
};

// Implementation for a given NativeRandomDistribution T, the required
// argument types for T constructors must match Args.
template <typename T, typename ... Args>
struct NRDProxyImpl : public NRDProxy
{
	NRDProxyImpl(boost::shared_ptr<T> dist) : mDistribution(dist) {}

	virtual bp::object operator()()
	{
		typename T::result_type v;
		mDistribution->next(v);
		return bp::object(v);
	}

	virtual bp::object operator()(size_t size)
	{
		pyublas::numpy_vector<typename T::result_type> v(size);
		mDistribution->next(v.begin(), v.end());
		return bp::object(v);
	}

	virtual dist_type get_distribution() const { return mDistribution; }

	// Helper to create a proxy for a random distribution with the given
	// parameters.
	static proxy_type create_proxy(const rng_type& rng, bp::list parameters)
	{
		boost::shared_ptr<T> dist = create_distribution(rng, parameters);
		return proxy_type(new NRDProxyImpl(dist));
	}

	// Helper to create the random distribution itself and to unpack and convert the
	// python parameter list.
	static boost::shared_ptr<T> create_distribution(const rng_type& rng, bp::list parameters)
	{
		return create_impl(rng, parameters);
	}


protected:
	boost::shared_ptr<T> mDistribution;

private:
	// This helper goes recursively through the given list of parameters and
	// unpacks them into CallArgs. At this point they are also converted from
	// python to the required C++ argument type.
	// If the given list of parameters is shorter than the required number
	// the recursion is aborted and the default arguments of the distribution
	// constructor are used.
	// If the length CallArgs matches the length of Args the recursion is
	// aborted by the non templated version below.
	template <typename... CallArgs>
	static boost::shared_ptr<T> create_impl(const rng_type& rng, bp::list parameters,
	                                        CallArgs... args)
	{
		const size_t pos = sizeof...(CallArgs);
		static_assert(
			sizeof...(CallArgs) < sizeof...(Args),
			"The recursion went over the maximum number of arguments");

		if (len(parameters) == sizeof...(CallArgs)) {
			// In complete parameters list, construct using default arguments.
			return boost::make_shared<T>(rng, args...);
		} else {
			// Unpack the next argument
			typedef typename ZTL::get<pos, ZTL::pack<Args...>>::type ArgType;
			ArgType new_value = bp::extract<ArgType>(parameters[pos]);
			return create_impl(rng, parameters, args..., new_value);
		}
	}

	// Terminial of recursion, if the amount of given parameters is longer
	// than the required parameters we throw an error
	static boost::shared_ptr<T> create_impl(const rng_type& rng, bp::list parameters, Args... args)
	{
		if (len(parameters) == sizeof...(Args)) {
			return boost::make_shared<T>(rng, args...);
		} else {
			throw std::invalid_argument("Invalid number of parameters");
		}
	}
};

// Factory function type for NRDProxyImpl instances
typedef std::function<std::unique_ptr<NRDProxy>(const NRDProxy::rng_type&, bp::list parameters)>
    NativeRandomDistributionFactory;

// Instantiate NRDProxyImpl classes using the TMP helper
typedef NRDProxy::Get<NativeUniformDistribution>::type UniformNRDProxy;
typedef NRDProxy::Get<NativeNormalDistribution>::type NormalNRDProxy;
typedef NRDProxy::Get<NativeRandIntDistribution>::type RandintNRDProxy;
typedef NRDProxy::Get<NativePoissonDistribution>::type PoissonNRDProxy;
typedef NRDProxy::Get<NativeExponentialDistribution>::type ExponentialNRDProxy;
typedef NRDProxy::Get<NativeBinomialDistribution>::type BinomialNRDProxy;

// Mapping from python names to factory functions
static NativeRandomDistributionFactory get_NRDProxy_factory(std::string name)
{
	static std::unordered_map<std::string, NativeRandomDistributionFactory> factories = {
	    {"uniform", UniformNRDProxy::create_proxy},
	    {"normal", NormalNRDProxy::create_proxy},
	    {"randint", RandintNRDProxy::create_proxy},
	    {"binomial", BinomialNRDProxy::create_proxy},
	    {"poisson", PoissonNRDProxy::create_proxy},
	    {"exponential", ExponentialNRDProxy::create_proxy}};

	auto it = factories.find(name);
	if (it == factories.end()) {
		throw std::invalid_argument("Unkown distribution");
	}
	return it->second;
}

static std::unique_ptr<NRDProxy> create_NRDProxy(std::string name, const NRDProxy::rng_type& rng,
                                                 bp::list parameters)
{
	NativeRandomDistributionFactory factory = get_NRDProxy_factory(name);
	return factory(rng, parameters);
}
}

PyNativeRNG::PyNativeRNG() :
	_impl(boost::make_shared<NativeRandomGenerator>())
{
}

PyNativeRNG::PyNativeRNG(size_t seed) :
	_impl(boost::make_shared<NativeRandomGenerator>(seed))
{
}

boost::shared_ptr<RandomGenerator> PyNativeRNG::_getRNG() const
{
	return _impl;
}

boost::shared_ptr<RandomDistribution>
PyNativeRNG::_getRandomDistribution(
        std::string distribution,
        bp::list parameters) const
{
	auto proxy = create_NRDProxy(distribution, _impl, parameters);
	return proxy->get_distribution();
}

bp::object PyNativeRNG::next(size_t n,
				std::string distribution,
				bp::list parameters,
				size_t /* mask_local */)
{
	std::unique_ptr<NRDProxy> dist(create_NRDProxy(distribution, _impl, parameters));
	if (n == 1)
	{
		return *dist->operator()();
	}
	else
	{
		return *dist->operator()(n);
	}
}

bp::list PyNativeRNG::permutation(bp::list const& lin)
{
    std::vector<bp::object> v;

    for(int i=0; i<bp::len(lin); i++)
        v.push_back(lin[i]);

    _impl->shuffle(v);

    bp::list lout;
    for(std::vector<bp::object>::iterator i=v.begin(); i<v.end(); i++)
        lout.append(*i);

    return lout;
}

distribution_int_t PyNativeRNG::randint(distribution_int_t a, distribution_int_t b)
{
	std::uniform_int_distribution<distribution_int_t> d(a, b);
	return d(*_impl);
}

distribution_int_t PyNativeRNG::randint(distribution_int_t b)
{
	std::uniform_int_distribution<distribution_int_t> d(b);
	return d(_impl->raw());
}

distribution_float_t PyNativeRNG::uniform(distribution_float_t a, distribution_float_t b)
{
	std::uniform_real_distribution<distribution_float_t> d(a, b);
	return d(_impl->raw());
}

distribution_float_t PyNativeRNG::normal(distribution_float_t mu, distribution_float_t std)
{
	std::normal_distribution<distribution_float_t> d(mu, std);
	return d(_impl->raw());
}

distribution_int_t PyNativeRNG::binomial(distribution_int_t n, distribution_float_t p)
{
	std::binomial_distribution<distribution_int_t> d(n, p);
	return d(_impl->raw());
}

distribution_int_t PyNativeRNG::poisson(distribution_float_t mean)
{
	std::poisson_distribution<distribution_int_t> d(mean);
	return d(_impl->raw());
}

distribution_float_t PyNativeRNG::exponential(distribution_float_t lambda)
{
	std::exponential_distribution<distribution_float_t> d(lambda);
	return d(_impl->raw());
}

PyNumpyRNG::PyNumpyRNG()
{
    NOT_IMPLEMENTED();
}

PyNumpyRNG::PyNumpyRNG(random_int_t)
{
    NOT_IMPLEMENTED();
}

PyGSLRNG::PyGSLRNG()
{
    NOT_IMPLEMENTED();
}

PyGSLRNG::PyGSLRNG(random_int_t)
{
    NOT_IMPLEMENTED();
}

PyRandomDistribution::PyRandomDistribution(
		std::string distribution,
		bp::list parameters,
		const PyAbstractRNG & rng,
		bp::object boundaries,
		std::string constrain
) {
	_impl = rng._getRandomDistribution(distribution, parameters);
	if(!_impl)
	{
		std::runtime_error("Invaild distribution for the passed RNG");
	}

	if(!boundaries.is_none())
	{
		RandomDistribution::ConstrainType c = RandomDistribution::NONE;
		if (constrain == "clip")
		{
			c = RandomDistribution::CLIP;
		}
		else if (constrain == "redraw")
		{
			c = RandomDistribution::REDRAW;
		}
		else
		{
			throw std::runtime_error("Invaild Constrain Type");
		}

		if (_impl->type() == RandomDistribution::INT)
		{
			_impl->setBoundaries(c,
			                     bp::extract<distribution_int_t>(boundaries[0]),
			                     bp::extract<distribution_int_t>(boundaries[1]) );
		}
		else
		{
			_impl->setBoundaries(c,
			                     bp::extract<distribution_float_t>(boundaries[0]),
			                     bp::extract<distribution_float_t>(boundaries[1]));
		}
	}
}

bp::object PyRandomDistribution::next(size_t n)
{
	if (n == 1)
	{
		if (_impl->type() == RandomDistribution::REAL)
		{
			distribution_float_t v;
			_impl->next(v);
			return bp::object(v);
		}
		else
		{
			NOT_IMPLEMENTED();
		}
	}
	else
	{
		if (_impl->type() == RandomDistribution::REAL)
		{
			pyublas::numpy_vector<distribution_float_t> v(n);
			std::vector<distribution_float_t> tmp(n);
			_impl->next(tmp);
			std::copy(tmp.begin(), tmp.end(), v.as_ublas().begin());
			return bp::object(v);
		}
		else
		{
			pyublas::numpy_vector<distribution_int_t> v(n);
			std::vector<distribution_int_t> tmp(n);
			_impl->next(tmp);
			std::copy(tmp.begin(), tmp.end(), v.as_ublas().begin());
			return bp::object(v);
		}
	}
}

boost::shared_ptr<RandomDistribution> PyRandomDistribution::_getDist() const
{
	return _impl;
}
