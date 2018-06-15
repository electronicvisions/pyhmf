#pragma once

#include "boost_python_fwd.h"

#include <map>
#include <string>
#include <vector>

#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/list.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/extract.hpp>
#if BOOST_VERSION < 105600
#include <boost/python/numeric.hpp>
#else
#include <boost/python/numpy.hpp>
#endif

/// Code required to initialize the boost python converters
void initializeCustomPythonConverters();

template<typename T>
struct MapFromPyDict {
	typedef std::map<std::string, T> MyDict;

	static void* convertible(PyObject * obj) {
		if (!PyDict_Check(obj)) return 0;
		bp::dict x = bp::dict(bp::object(bp::handle<>(bp::borrowed(obj))));
		bp::list keys = x.keys();
		for (int i = 0; i < bp::len(keys); i++) {
			bp::extract<std::string> key(keys[i]);
			bp::extract<T> value(x[keys[i]]);
			if (!(key.check() && value.check()))
				return 0;
		}
		return obj;
	}

	static void construct(PyObject * obj,
	                      bp::converter::rvalue_from_python_stage1_data * data) {

		MyDict ret;

		// extract keys and fill map key-wise
		bp::dict x = bp::dict(bp::object(bp::handle<>(bp::borrowed(obj))));

		bp::list keys = x.keys();
		for (int i = 0; i < bp::len(keys); i++) {
			std::string key = bp::extract<std::string>(keys[i]);
			ret[key] = bp::extract<T>(x[keys[i]]);
		}

		void * storage = ((bp::converter::rvalue_from_python_storage<MyDict>*)data)->storage.bytes;
		new (storage) std::map<std::string, T>(ret);

		data->convertible = storage;
	}

	MapFromPyDict() {
		bp::converter::registry::push_back(&convertible, &construct, bp::type_id<MyDict>());
	}
};

template<typename T>
struct VectorFromPyList {
	typedef std::vector<T> MyVector;

	static void* convertible(PyObject * obj) {
		if (!PyList_Check(obj)) return 0;
		return obj;
	}

	static void construct(PyObject * obj,
	                      bp::converter::rvalue_from_python_stage1_data * data) {


		bp::object o = bp::object(bp::handle<>(bp::borrowed(obj)));
		MyVector ret(bp::len(o));

		// extract vector
		for(int i = 0; i < bp::len(o); i++) {
			ret[i] = bp::extract<T>(o[i]);
		}

		void * storage = ((bp::converter::rvalue_from_python_storage<MyVector>*)data)->storage.bytes;
		new (storage) std::vector<T>(ret);
		data->convertible = storage;
	}

	VectorFromPyList() {
		bp::converter::registry::push_back(&convertible, &construct, bp::type_id<MyVector>());
	}
};

