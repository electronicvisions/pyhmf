#pragma once

#include <string>
#include "pyhmf/boost_python_fwd.h"
#include "pyublas.h"
#include <Python.h>
#include "euter/space.h"

class Structure;

class PySpace
{
public:
	PySpace(
			std::string axes="xyz",
			double scale_factor=1.0,
			bp::object offset=SentinelKeeper::emptyPyObject,
			bp::object periodic_boundaries=SentinelKeeper::emptyPyObject
		   );
	py_matrix_type distances(py_vector_type A, py_vector_type B, bool expand=false);

	boost::shared_ptr<Space> _impl;
};

class PyStructure
{
public:
	virtual ~PyStructure() = 0;
    py_vector_type generate_positions(size_t n);

    boost::shared_ptr<Structure> _impl;
};

class PyLine : public PyStructure
{
public:
    PyLine(double dx=1.0,
           double x0=0.0,
           double y=0.0,
           double z=0.0,
           std::string fill_order="sequential"
          );
};

class PyGrid2D : public PyStructure
{
public:
    PyGrid2D(double aspect_ratio=1.0,
             double dx=1.0,
             double dy=1.0,
             double x0=0.0,
             double y0=0.0,
             double z=0.0,
             std::string fill_order="sequential"
            );
};

class PyGrid3D : public PyStructure
{
public:
    PyGrid3D(double aspect_ratioXY=1.0,
             double aspect_ratioXZ=1.0,
             double dx=1.0,
             double dy=1.0,
             double dz=1.0,
             double x0=0.0,
             double y0=0.0,
             double z0=0.0,
             std::string fill_order="sequential"
            );
};

class PyShape
{
public:
	virtual ~PyShape() = 0;
    py_vector_type sample(size_t n);
    
    #ifndef PYPLUSPLUS
    boost::shared_ptr<Shape> _impl;
    #endif
};

class PyCuboid : public PyShape
{
public:
    PyCuboid(double width,
           double height,
           double depth
          );
};

class PySphere : public PyShape
{
public:
	PySphere(double radius);
};

class PyRandomStructure : public PyStructure
{
public:
	PyRandomStructure(PyShape& boundary, bp::tuple origin/*, PyRandomGenerator rng=NONE*/);
};
