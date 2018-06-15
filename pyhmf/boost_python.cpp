#include "boost_python.h"

void initializeCustomPythonConverters()
{
	MapFromPyDict<double>();
	MapFromPyDict<bp::object>();
	VectorFromPyList<double>();
}
