#include <boost/make_shared.hpp>
#include "py_space.h"
#include "euter/exceptions.h"
#include "errors.h"

#include <boost/numeric/ublas/io.hpp>

using namespace euter;

PySpace::PySpace(
		std::string axes,
		double scale_factor,
		bp::object offset,
		bp::object periodic_boundaries
		)
{
	// Extract axes from string
	std::bitset<3> tmp_axes(0);
	if(axes.find('x') != std::string::npos)
		tmp_axes |= Space::Axis::X;
	if(axes.find('y') != std::string::npos)
		tmp_axes |= Space::Axis::Y;
	if(axes.find('z') != std::string::npos)
		tmp_axes |= Space::Axis::Z;

	// Extract offset vector
	SpatialTypes::coord_type tmp_offset;
	if(offset.ptr() == SentinelKeeper::emptyPyObject.ptr())  // mimic python is operator
	{
		for(size_t i=0; i<tmp_offset.size(); ++i)
			tmp_offset[i] = 0;
	}
	else if(bp::extract<double>(offset).check())
	{
		double dbl_offset = bp::extract<double>(offset);
		tmp_offset(0) = dbl_offset;
		tmp_offset(1) = dbl_offset;
		tmp_offset(2) = dbl_offset;
	}
	else if(bp::extract<py_vector_type>(offset).check())
	{
		py_vector_type npy_offset = bp::extract<py_vector_type>(offset);
		tmp_offset = npy_offset.as_ublas();
	}
	else
	{
		NOT_IMPLEMENTED();
	}

	// Extract boundaries. Yay. Fun.
	SpatialTypes::Boundaries boundaries;
	double nan = std::numeric_limits<SpatialTypes::distance_type>::quiet_NaN();

	if(periodic_boundaries.ptr() == SentinelKeeper::emptyPyObject.ptr())
	{
		boundaries = SpatialTypes::Boundaries();
	}
	else if(bp::extract<bp::tuple>(periodic_boundaries).check())
	{
		bp::tuple tpl_boundaries = bp::extract<bp::tuple>(periodic_boundaries);
		assert(bp::len(tpl_boundaries) == 3);

		for(size_t i=0; i<=2; ++i)
		{
			if(bp::object(tpl_boundaries[i]).ptr() == SentinelKeeper::emptyPyObject.ptr())
			{
				boundaries(i) = std::make_pair(nan, nan);
			}
			else
			{
				assert(bp::len(tpl_boundaries[i]) == 2);
				boundaries(i) = std::make_pair<double, double>(
					bp::extract<double>(tpl_boundaries[i][0])(),
					bp::extract<double>(tpl_boundaries[i][1])());
			}
		}
	}

	_impl = boost::make_shared<Space>(tmp_axes, scale_factor, tmp_offset, boundaries);
}

py_matrix_type PySpace::distances(py_vector_type A, py_vector_type B, bool expand)
{
	if(expand)
		NOT_IMPLEMENTED();

	ublas::vector<SpatialTypes::coord_type> a(A.dims()[1]);
	ublas::vector<SpatialTypes::coord_type> b(B.dims()[1]);

	for(int i=0; i<A.dims()[1]; ++i)
	{
		a[i] = SpatialTypes::coord_type();
		for(int j=0; j<A.dims()[0]; ++j)
			a[i][j] = A.sub(j, i);
	}
	
	for(int i=0; i<A.dims()[1]; ++i)
	{
		b[i] = SpatialTypes::coord_type();
		for(int j=0; j<B.dims()[0]; ++j)
			b[i][j] = B.sub(j, i);
	}
	
	return _impl->distances(a, b);
}


PyStructure::~PyStructure() {}

py_vector_type PyStructure::generate_positions(size_t n)
{
	py_vector_type positions(3*n);
	auto raw_positions = _impl->generatePositions(n);

	size_t i = 0;
	for(auto vec : raw_positions)
	{
		positions[0*n + i] = vec[0];
		positions[1*n + i] = vec[1];
		positions[2*n + i] = vec[2];
		i++;
	}
	
	const long shape[2] = {3, -1};
	positions.reshape(2, shape);
	
	return positions;
}

PyLine::PyLine(double dx,
               double x0,
               double y,
               double z,
               std::string fill_order)
{
    SpatialTypes::coord_type offset;
    offset(0) = x0;
    offset(1) = y;
    offset(2) = z;

    Structure::FillOrder fo;
    if(fill_order == "sequential")
    {
        fo = Structure::FillOrder::Sequential;
    }
    else if(fill_order == "random")
    {
        fo = Structure::FillOrder::Random;
    }
    else
    {
        throw std::runtime_error("'fill_order' must be either \"sequential\" or \"random\"!");
    }

    Line impl(dx, offset, fo);
    _impl = boost::make_shared<Line>(impl);
}

PyGrid2D::PyGrid2D(double aspect_ratio,
                   double dx,
                   double dy,
                   double x0,
                   double y0,
                   double z,
                   std::string fill_order
                   )
{
    SpatialTypes::coord_type offset;
    offset(0) = x0;
    offset(1) = y0;
    offset(2) = z;

    Structure::FillOrder fo;
    if(fill_order == "sequential")
    {
        fo = Structure::FillOrder::Sequential;
    }
    else if(fill_order == "random")
    {
        fo = Structure::FillOrder::Random;
    }
    else
    {
        throw std::runtime_error("'fill_order' must be either \"sequential\" or \"random\"!");
    }

    _impl = boost::make_shared<Grid2D>(dx, dy, aspect_ratio, offset, fo);
}

PyGrid3D::PyGrid3D(double aspect_ratioXY,
                   double aspect_ratioXZ,
                   double dx,
                   double dy,
                   double dz,
                   double x0,
                   double y0,
                   double z0,
                   std::string fill_order
                   )
{
    SpatialTypes::coord_type offset;
    offset(0) = x0;
    offset(1) = y0;
    offset(2) = z0;

    Structure::FillOrder fo;
    if(fill_order == "sequential")
    {
        fo = Structure::FillOrder::Sequential;
    }
    else if(fill_order == "random")
    {
        fo = Structure::FillOrder::Random;
    }
    else
    {
        throw std::runtime_error("'fill_order' must be either \"sequential\" or \"random\"!");
    }

    Grid3D impl(dx, dy, dz, aspect_ratioXY, aspect_ratioXZ, offset, fo);
    _impl = boost::make_shared<Grid3D>(impl);
}

PyShape::~PyShape()
{
}

py_vector_type PyShape::sample(size_t n)
{
	py_vector_type positions(3*n);
	auto raw_positions = _impl->sample(n);

	size_t i = 0;
	for(auto vec : raw_positions)
	{
		positions[0*n + i] = vec[0];
		positions[1*n + i] = vec[1];
		positions[2*n + i] = vec[2];
		i++;
	}
	
	const long shape[2] = {3, -1};
	positions.reshape(2, shape);
	
	return positions;
}

PyCuboid::PyCuboid(double width, double height, double depth)
{
    Cuboid impl(width, height, depth);
    _impl = boost::make_shared<Cuboid>(impl);
}

PySphere::PySphere(double radius)
{
    Sphere impl(radius);
    _impl = boost::make_shared<Sphere>(impl);
}

PyRandomStructure::PyRandomStructure(PyShape& boundary, bp::tuple origin/*, PyRandomGenerator rng=NONE*/)
{
	if(bp::len(origin) != 3)
		throw std::runtime_error("Origin must have three dimensions!");

	SpatialTypes::coord_type offset;
    offset(0) = bp::extract<double>(origin[0]);
    offset(1) = bp::extract<double>(origin[1]);
    offset(2) = bp::extract<double>(origin[2]);

	_impl = boost::make_shared<RandomStructure>(boundary._impl, offset);
}
