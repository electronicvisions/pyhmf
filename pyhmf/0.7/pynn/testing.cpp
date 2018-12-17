#include "testing.h"

#include <sstream>
#if BOOST_VERSION < 105600
#include <boost/python/numeric.hpp>
#else
#include <boost/python/numpy.hpp>
#endif

#include "pyhmf/objectstore.h"

#include "errors.h"
#include "euter/celltypes.h"

namespace testing
{

void numpyExample(py_vector_type o) {
	o(0) = 2.0;
}

size_t getObjectStoreSize()
{
	euter::ObjectStore& os = getStore();
	std::stringstream store;
	boost::archive::binary_oarchive oa(store);
	oa << os;
	return store.str().size();
}

void test_InvalidParameterValueError()
{
    throw PyInvalidParameterValueError("Hello sweet kitty!");
}

void test_NonExistentParameterError()
{
    throw PyNonExistentParameterError("my_param", "my_model", {"p0", "p1", "p2"});
}

void test_InvalidDimensionsError()
{
    throw PyInvalidDimensionsError("Hello sweet kitty!");
}

void test_ConnectionError()
{
    throw PyConnectionError("Hello sweet kitty!");
}

void test_InvalidModelError()
{
    throw PyInvalidModelError("Hello sweet kitty!");
}

void test_NoModelAvailableError()
{
    throw PyNoModelAvailableError("Hello sweet kitty!");
}

void test_NothingToWriteError()
{
    throw PyNothingToWriteError("Hello sweet kitty!");
}

void test_InvalidWeightError()
{
    throw PyInvalidWeightError("Hello sweet kitty!");
}

void test_NotLocalError()
{
    throw PyNotLocalError("Hello sweet kitty!");
}

void test_RecordingError()
{
    throw PyRecordingError("foobar", euter::CellType(0));
}
}
