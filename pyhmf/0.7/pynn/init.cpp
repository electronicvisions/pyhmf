#include "init.h"

#include "py_models.h"
#include "errors.h"
#include "pyhmf/boost_python.h"
#include <pyublas/converters.hpp>

#include <log4cxx/basicconfigurator.h>

// Expands to (for elm being IF_brainscales_hardware):
// bp::scope().attr("IF_brainscales_hardware") = pynnCelltypes.attr("IF_brainscales_hardware");
// bp::object(bp::scope().attr("IF_brainscales_hardware")).attr("__module__") = "pyhmf";
#define COPY_FROM_PYNN(mod, elm) \
    bp::scope().attr(BOOST_PP_STRINGIZE(elm)) = mod.attr(BOOST_PP_STRINGIZE(elm)); \
    bp::object(bp::scope().attr(BOOST_PP_STRINGIZE(elm))).attr("__module__") = "pyhmf";

void _loadPyNNFileTypes()
{
	bp::object pynn_files = bp::import("pyNN.recording.files");

	COPY_FROM_PYNN(pynn_files, StandardTextFile);
	COPY_FROM_PYNN(pynn_files, PickleFile);
	COPY_FROM_PYNN(pynn_files, NumpyBinaryFile);
}

void initialize()
{
	// we use the format PYNNVERSION-SHA1CHECKSUM
	bp::scope().attr("__version__") = std::string("0.7.5-") + bp::str(BOOST_PP_STRINGIZE(VERSION));

	log4cxx::BasicConfigurator::resetConfiguration();
	log4cxx::BasicConfigurator::configure();

	initializeCustomPythonConverters();

	_loadPyNNFileTypes();
	_loadPyNNCellTypes();
	_loadPyNNErrors();
}
