#pragma once

// Provides Py++ compatible definitions

#ifndef PYPLUSPLUS
#include <boost/python/object_fwd.hpp>
#include <boost/python.hpp>
#else
namespace boost { namespace python {
class object;
}}
#endif

namespace boost {
    namespace python {
        class dict;
        class list;
        class slice;
        class tuple;
        class str;
        
        object import(str);
        
#if BOOST_VERSION < 105600
        namespace numeric
        {
            class array;
        }
#else
        namespace numpy
        {
            class ndarray;
        }
#endif
    }
}

namespace bp = boost::python;
#if BOOST_VERSION < 105600
typedef bp::numeric::array npyarray;
#else
typedef bp::numpy::ndarray npyarray;
#endif

/// Workaround to provide default parameters, inherit from this class to
/// use the objects as default values for member functions
/// @note This is needed to work around the py++ parser limitations
struct SentinelKeeper {
	/// Sentinel boost::python::object to be used as default parameter
	static const bp::object emptyPyObject;
	/// Sentinel boost::python::dict to be used as default parameter
	static const bp::dict emptyPyDict;
	/// Sentinel boost::python::list to be used as default parameter
	static const bp::list emptyPyList;
	/// Sentinel boost::python::tuple to be used as default parameter
	static const bp::tuple emptyPyTuple;
};
