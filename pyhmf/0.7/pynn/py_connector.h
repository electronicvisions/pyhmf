#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

#include "pyhmf/boost_python_fwd.h"
#include "pyublas.h"

#include "py_random.h"

class Connector;
class AllToAllConnector;
class OneToOneConnector;
class FixedProbabilityConnector;
class FixedNumberPreConnector;
class FixedNumberPostConnector;
class DistanceDependentProbabilityConnector;
class FromListConnector;

struct PyConnector : public SentinelKeeper
{
	typedef double value_type;
	typedef py_vector_type vector_type;
	typedef py_matrix_type matrix_type;
	typedef PyRandomDistribution rng_type;

	virtual boost::shared_ptr<Connector> _getImpl() = 0;
};

class PyAllToAllConnector : public PyConnector {
public:
	PyAllToAllConnector(boost::shared_ptr<AllToAllConnector>);
	static boost::shared_ptr<PyAllToAllConnector>
	create(
		bool       const allow_self_connections = true,
		bp::object const weights = emptyPyObject,
		bp::object const delays  = emptyPyObject
	);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<AllToAllConnector> impl;
};


class PyOneToOneConnector : public PyConnector {
public:
	PyOneToOneConnector(boost::shared_ptr<OneToOneConnector>);
	static boost::shared_ptr<PyOneToOneConnector>
	create(
		bool       const allow_self_connections = true,
		bp::object const weights = emptyPyObject,
		bp::object const delays  = emptyPyObject
	);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<OneToOneConnector> impl;
};


class PyFixedProbabilityConnector : public PyConnector {
public:
	PyFixedProbabilityConnector(boost::shared_ptr<FixedProbabilityConnector>);
	static boost::shared_ptr<PyFixedProbabilityConnector>
	create(
		double     const p_connect,
		bool       const allow_self_connections = true,
		bp::object const weights = emptyPyObject,
		bp::object const delays  = emptyPyObject
	);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<FixedProbabilityConnector> impl;
};

class PyFixedNumberPreConnector : public PyConnector {
public:
	PyFixedNumberPreConnector(boost::shared_ptr<FixedNumberPreConnector>);
	static boost::shared_ptr<PyFixedNumberPreConnector>
	create(
		size_t     const n,
		bool       const allow_self_connections = true,
		bp::object const weights = emptyPyObject,
		bp::object const delays  = emptyPyObject
	);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<FixedNumberPreConnector> impl;
};

class PyFixedNumberPostConnector : public PyConnector {
public:
	PyFixedNumberPostConnector(boost::shared_ptr<FixedNumberPostConnector>);
	static boost::shared_ptr<PyFixedNumberPostConnector>
	create(
		size_t     const n,
		bool       const allow_self_connections = true,
		bp::object const weights = emptyPyObject,
		bp::object const delays  = emptyPyObject
	);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<FixedNumberPostConnector> impl;
};

class ExpressionBasedProbabilityGenerator;

class PyDistanceDependentProbabilityConnector : public PyConnector {
public:
	PyDistanceDependentProbabilityConnector(boost::shared_ptr<DistanceDependentProbabilityConnector>);
	static boost::shared_ptr<PyDistanceDependentProbabilityConnector>
	create(
		bp::str    const d_expression,
		bool       const allow_self_connections = true,
		bp::object const weights = emptyPyObject,
		bp::object const delays = emptyPyObject,
		bp::object const space = emptyPyObject,
		bool       const safe = true,
		bool       const verbose = false,
		int        const n_connections = -1
	);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<DistanceDependentProbabilityConnector> impl;
};

class PyFromListConnector : public PyConnector {
public:
	PyFromListConnector(boost::shared_ptr<FromListConnector>);
	static boost::shared_ptr<PyFromListConnector> create(bp::object conn_list);

	virtual boost::shared_ptr<Connector> _getImpl();

	boost::shared_ptr<FromListConnector> impl;
};
