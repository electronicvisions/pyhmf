#pragma once

#include <pywrap/compat/numpy.hpp>

typedef pyublas::numpy_matrix<double, boost::numeric::ublas::row_major> py_matrix_type;
typedef pyublas::numpy_vector<double> py_vector_type;
