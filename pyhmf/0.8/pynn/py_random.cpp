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

struct NRDProxy
{
	typedef boost::shared_ptr<NativeRandomGenerator> rng_type;
	typedef NativeRandomGenerator::rng_type raw_rng_type;

	virtual bp::object operator()(raw_rng_type & rng, bp::list list)
	{
		setDefaults(list);
		switch(bp::len(list))
		{
		case 0:
			return this->operator ()(rng);
		case 1:
			return this->operator ()(rng, list[0]);
		case 2:
			return this->operator ()(rng, list[0], list[1]);
		default:
		{ throw std::invalid_argument("Invalid number or parameters"); }
		}
	}

	virtual bp::object operator()(size_t size, raw_rng_type & rng, bp::list list)
	{
		setDefaults(list);
		switch(bp::len(list))
		{
		case 0:
			return this->operator ()(size, rng);
		case 1:
			return this->operator ()(size, rng, list[0]);
		case 2:
			return this->operator ()(size, rng, list[0], list[1]);
		default:
		{ throw std::invalid_argument("Invalid number or parameters"); }
		}
	}

	virtual RandomDistribution * create(const rng_type & rng, bp::list list)
	{
		setDefaults(list);
		switch(bp::len(list))
		{
		case 0:
			return this->create(rng);
		case 1:
			return this->create(rng, list[0]);
		case 2:
			return this->create(rng, list[0], list[1]);
		default:
		{ throw std::invalid_argument("Invalid number or parameters"); }
		}
	}

	virtual void setDefaults(bp::list & ) const {}

	virtual bp::object operator()(raw_rng_type &)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual bp::object operator()(raw_rng_type &, bp::object)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual bp::object operator()(raw_rng_type &, bp::object, bp::object)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual bp::object operator()(size_t, raw_rng_type &)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual bp::object operator()(size_t, raw_rng_type &, bp::object)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual bp::object operator()(size_t, raw_rng_type &, bp::object, bp::object)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual RandomDistribution * create(const rng_type &)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual RandomDistribution * create(const rng_type &, bp::object)
	{ throw std::invalid_argument("Invalid number or parameters"); }

	virtual RandomDistribution * create(const rng_type &, bp::object, bp::object)
	{ throw std::invalid_argument("Invalid number or parameters"); }
};

template <typename T, typename ... Args>
struct NRDProxyImpl : public NRDProxy
{
	typedef typename T::dist_type dist_type;

	virtual bp::object operator()(raw_rng_type & rng,
								  typename object_type<Args>::type ... py_params)
	{
		typename dist_type::param_type t(help_extract<Args>(py_params)...);
		return bp::object(mDistribution(rng, t));
	}

	virtual bp::object operator()(size_t size, raw_rng_type & rng,
								  typename object_type<Args>::type ... py_params)
	{
		typename dist_type::param_type t(help_extract<Args>(py_params)...);
		pyublas::numpy_vector<typename T::result_type> v(size);
		for(auto it = v.as_ublas().begin(), iend = v.as_ublas().end(); it != iend; ++it)
		{
			*it = mDistribution(rng, t);
		}
		return bp::object(v);
	}

	virtual RandomDistribution * create(const rng_type & rng,
								  typename object_type<Args>::type ... py_params)
	{
		return new T(rng, help_extract<Args>(py_params)...);
	}

protected:
	dist_type mDistribution;
};

struct UniformNRDProxy : NRDProxyImpl<NativeUniformDistribution, distribution_float_t, distribution_float_t>
{
	virtual void setDefaults(bp::list & list) const
	{
		if (len(list) == 0)
			list.append(0.0);
		if (len(list) == 1)
			list.append(1.0);
	}
};

struct NormalNRDProxy : NRDProxyImpl<NativeNormalDistribution, distribution_float_t, distribution_float_t>
{
	virtual void setDefaults(bp::list & list) const
	{
		if (len(list) == 0)
			list.append(0.0);
		if (len(list) == 1)
			list.append(1.0);
	}
};

struct RandintNRDProxy : NRDProxyImpl<NativeRandIntDistribution, distribution_int_t, distribution_int_t>
{
	virtual void setDefaults(bp::list & list) const
	{
		if (len(list) == 1)
			list.insert(0, 0);
	}
};

typedef NRDProxyImpl<NativeBinomialDistribution, distribution_int_t, distribution_float_t> BinomialNRDProxy;

struct PoissonNRDProxy : NRDProxyImpl<NativePoissonDistribution, distribution_float_t>
{
	virtual void setDefaults(bp::list & list) const
	{
		if (len(list) == 0)
			list.append(1.0);
	}
};

struct ExponentialNRDProxy : NRDProxyImpl<NativeExponentialDistribution, distribution_float_t>
{
	virtual void setDefaults(bp::list & list) const
	{
		if (len(list) == 0)
			list.append(1.0);
	}
};

std::unordered_map< std::string, std::unique_ptr<NRDProxy> > create_native_distributions()
{
	std::unordered_map< std::string, std::unique_ptr<NRDProxy> > map;
	map["uniform"] = std::unique_ptr<UniformNRDProxy>(new UniformNRDProxy);
	map["normal"] = std::unique_ptr<NormalNRDProxy>(new NormalNRDProxy);
	map["randint"] = std::unique_ptr<RandintNRDProxy>(new RandintNRDProxy);
	map["binomial"] = std::unique_ptr<BinomialNRDProxy>(new BinomialNRDProxy);
	map["poisson"] = std::unique_ptr<PoissonNRDProxy>(new PoissonNRDProxy);
	map["exponential"] = std::unique_ptr<ExponentialNRDProxy>(new ExponentialNRDProxy);

	return map;
}

NRDProxy & getNativeDistribution(std::string name)
{
	static std::unordered_map< std::string, std::unique_ptr<NRDProxy> > d = create_native_distributions();
	auto it = d.find(name);
	if(it == d.end())
	{
		throw std::invalid_argument("Unkown distribution");
	}
	return *(it->second);
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
	return boost::shared_ptr<RandomDistribution>(
				getNativeDistribution(distribution).create(_impl, parameters));
}

bp::object PyNativeRNG::next(size_t n,
				std::string distribution,
				bp::list parameters,
				size_t /* mask_local */)
{
	if (n == 1)
	{
		return getNativeDistribution(distribution)(_impl->raw(), parameters);
	}
	else
	{
		return getNativeDistribution(distribution)(n, _impl->raw(), parameters);
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
