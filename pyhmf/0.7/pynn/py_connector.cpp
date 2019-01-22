#include <functional>

#include "py_connector.h"
#include "py_space.h"

#include <boost/make_shared.hpp>

#include "pyhmf/boost_python.h"

#include "euter/connector.h"
#include "euter/alltoallconnector.h"
#include "euter/onetooneconnector.h"
#include "euter/fixedprobabilityconnector.h"
#include "euter/fixednumberpreconnector.h"
#include "euter/fixednumberpostconnector.h"
#include "euter/distancedependentprobabilityconnector.h"
#include "euter/fromlistconnector.h"

namespace
{

Connector::default_type getDefault(bp::object const obj)
{
	if(obj.is_none())
	{
		return Connector::default_type(0.0);
	}
	else
	{
		bp::extract<PyConnector::value_type> scalar(obj);
		if (scalar.check())
		{
			return scalar();
		}

		bp::extract<PyConnector::matrix_type> matrix(obj);
		if (matrix.check())
		{
			return matrix();
		}

		bp::extract<PyConnector::vector_type> vector(obj);
		if (vector.check())
		{
			return vector();
		}

		bp::extract<PyConnector::rng_type> rng(obj);
		if (rng.check())
		{
			return rng()._getDist();
		}
	}
	throw std::runtime_error("Invalid type"); // TODO be more verbose
}

}


PyAllToAllConnector::PyAllToAllConnector(
	boost::shared_ptr<AllToAllConnector> impl
) : impl(impl) {}

// TODO remove after a reasonable amount of functions is implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

boost::shared_ptr<PyAllToAllConnector>
PyAllToAllConnector::create(
	bool       const allow_self_connections,
	bp::object const weights,
	bp::object const delays
) {
	auto impl = boost::make_shared<AllToAllConnector>(
	            allow_self_connections,
	            getDefault(weights),
	            getDefault(delays) );
	return boost::make_shared<PyAllToAllConnector>(impl);
}

boost::shared_ptr<Connector> PyAllToAllConnector::_getImpl()
{
	return impl;
}

PyOneToOneConnector::PyOneToOneConnector(
	boost::shared_ptr<OneToOneConnector> impl
) : impl(impl) {}

boost::shared_ptr<PyOneToOneConnector>
PyOneToOneConnector::create(
	bool       const allow_self_connections,
	bp::object const weights,
	bp::object const delays
) {
	auto impl = boost::make_shared<OneToOneConnector>(
	            allow_self_connections,
	            getDefault(weights),
	            getDefault(delays) );
	return boost::make_shared<PyOneToOneConnector>(impl);
}

boost::shared_ptr<Connector> PyOneToOneConnector::_getImpl()
{
	return impl;
}

PyFixedProbabilityConnector::PyFixedProbabilityConnector(
	boost::shared_ptr<FixedProbabilityConnector> impl
) : impl(impl) {}

boost::shared_ptr<PyFixedProbabilityConnector>
PyFixedProbabilityConnector::create(
	double     const p_connect,
	bool       const allow_self_connections,
	bp::object const weights,
	bp::object const delays
) {
	auto impl = boost::make_shared<FixedProbabilityConnector>(
	            p_connect,
	            allow_self_connections,
	            getDefault(weights),
	            getDefault(delays) );
	return boost::make_shared<PyFixedProbabilityConnector>(impl);
}

boost::shared_ptr<Connector> PyFixedProbabilityConnector::_getImpl()
{
	return impl;
}

PyFixedNumberPreConnector::PyFixedNumberPreConnector(
        boost::shared_ptr<FixedNumberPreConnector> impl
        ) : impl(impl) {}

boost::shared_ptr<PyFixedNumberPreConnector>
PyFixedNumberPreConnector::create(
        size_t      const n, // TODO: support RandomDistribution
        bool        const allow_self_connections,
        bp::object  const weights,
        bp::object  const delays
) {
	auto impl = boost::make_shared<FixedNumberPreConnector>(
	            n,
	            allow_self_connections,
	            getDefault(weights),
	            getDefault(delays) );
	return boost::make_shared<PyFixedNumberPreConnector>(impl);
}

boost::shared_ptr<Connector> PyFixedNumberPreConnector::_getImpl()
{
	return impl;
}

PyFixedNumberPostConnector::PyFixedNumberPostConnector(
        boost::shared_ptr<FixedNumberPostConnector> impl
        ) : impl(impl) {}

boost::shared_ptr<PyFixedNumberPostConnector>
PyFixedNumberPostConnector::create(
        size_t      const n, // TODO: support RandomDistribution
        bool        const allow_self_connections,
        bp::object  const weights,
        bp::object  const delays
) {
	auto impl = boost::make_shared<FixedNumberPostConnector>(
	            n,
	            allow_self_connections,
	            getDefault(weights),
	            getDefault(delays) );
	return boost::make_shared<PyFixedNumberPostConnector>(impl);
}

boost::shared_ptr<Connector> PyFixedNumberPostConnector::_getImpl()
{
	return impl;
}

PyDistanceDependentProbabilityConnector::PyDistanceDependentProbabilityConnector(
        boost::shared_ptr<DistanceDependentProbabilityConnector> impl
        ) : impl(impl) {}

class ExpressionBasedProbabilityGenerator : public ProbabilityGenerator
{
public:
    ExpressionBasedProbabilityGenerator(bp::str expression) : expression(expression) {}
	virtual ConnectorTypes::value_type operator()(SpatialTypes::distance_type distance) const
	{
        bp::object np = bp::import("numpy");
		bp::object globals = np.attr("__dict__");
		bp::dict locals;
        locals["d"] = distance;

		return bp::extract<ConnectorTypes::value_type>(bp::eval(expression, globals, locals));
	}

	virtual ConnectorTypes::matrix_type operator()(
	    ublas::matrix<SpatialTypes::distance_type> const& distances) const
	{
		bp::object np = bp::import("numpy");
		bp::object globals = np.attr("__dict__");
		bp::dict locals;
		locals["d"] = py_matrix_type(distances);

		locals["p"] = bp::eval(expression, globals, locals);

		// handle case when expression evaluation returns a bool ndarray (e.g. "d<3")
		bp::exec("p = p.astype(float, copy=False)", globals, locals);
		py_matrix_type p = bp::extract<py_matrix_type>(locals["p"]);
		return p.as_ublas();
	}

private:
    bp::str expression;
};

boost::shared_ptr<PyDistanceDependentProbabilityConnector>
PyDistanceDependentProbabilityConnector::create(
		bp::str     const d_expression,
		bool        const allow_self_connections,
		bp::object  const weights,
		bp::object  const delays,
		bp::object  const space,
		bool        const safe,
		bool        const verbose,
		int         const n_connections
		)
{
	boost::shared_ptr<Space> tmp_space;
	if(bp::extract<PySpace>(space).check())
	{
		tmp_space = static_cast<PySpace>(bp::extract<PySpace>(space))._impl;
	}
	else
	{
		tmp_space = boost::make_shared<Space>();
	}

    auto impl = boost::make_shared<DistanceDependentProbabilityConnector>(
			boost::make_shared<ExpressionBasedProbabilityGenerator>(d_expression),
			allow_self_connections,
			getDefault(weights),
			getDefault(delays),
			tmp_space,
			n_connections
			);
    return boost::make_shared<PyDistanceDependentProbabilityConnector>(impl);
}

boost::shared_ptr<Connector> PyDistanceDependentProbabilityConnector::_getImpl()
{
	return impl;
}

PyFromListConnector::PyFromListConnector(
	boost::shared_ptr<FromListConnector> impl
) : impl(impl) {}

boost::shared_ptr<PyFromListConnector>
PyFromListConnector::create(bp::object conn_list) {
	size_t size = bp::len(conn_list);
	PyConnector::vector_type delays(size, 0.0), weigths(size, 0.0);
	std::vector<FromListConnector::Connection> connections(size);
	for(size_t ii = 0; ii < size; ++ii)
	{
		size_t from = bp::extract<size_t>(conn_list[ii][0]);
		size_t to = bp::extract<size_t>(conn_list[ii][1]);
		connections[ii] = FromListConnector::Connection{from,to};
		if(len(conn_list[ii]) >= 3)
		{
			weigths[ii] = bp::extract<value_type>(conn_list[ii][2]);
		}
		if(len(conn_list[ii]) >= 4)
		{
			delays[ii] = bp::extract<value_type>(conn_list[ii][3]);
		}
	}
    auto impl = boost::make_shared<FromListConnector>(
	            std::move(connections), std::move(weigths), std::move(delays));
	return boost::make_shared<PyFromListConnector>(impl);
}

boost::shared_ptr<Connector> PyFromListConnector::_getImpl()
{
	return impl;
}
