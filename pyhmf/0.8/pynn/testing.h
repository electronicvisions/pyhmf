/// This header provides functions for type conversions lowlevel tests

// Py++ compatible boost python stuff
#include <cstring>

#include "pyublas.h"
#include "pyhmf/boost_python_fwd.h"
#include "errors.h"

namespace testing
{
/// Does 'o[0] = 2.0' on the input array
void numpyExample(py_vector_type o);

// Returns the size of the (binaries) serialized objectstore in bytes 
size_t getObjectStoreSize();

void test_InvalidParameterValueError();
void test_NonExistentParameterError();
void test_InvalidDimensionsError();
void test_ConnectionError();
void test_InvalidModelError();
void test_NoModelAvailableError();
void test_NothingToWriteError();
void test_InvalidWeightError();
void test_NotLocalError();
void test_RecordingError();
}
