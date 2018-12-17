#include <iostream>
#include "errors.h"
#include "euter/celltypes.h"
#include "py_models.h"

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define ERROR_SEQ BOOST_PP_VARIADIC_TO_SEQ(\
    InvalidParameterValueError, NonExistentParameterError, \
    InvalidDimensionsError, ConnectionError, InvalidModelError, NoModelAvailableError, \
    RoundingWarning, NothingToWriteError, InvalidWeightError, NotLocalError, \
    RecordingError)

// Expands to (for elm being IF_brainscales_hardware):
// bp::scope().attr("IF_brainscales_hardware") = pynnCelltypes.attr("IF_brainscales_hardware");
// bp::object(bp::scope().attr("IF_brainscales_hardware")).attr("__module__") = "pyhmf";
#define COPY_FROM_PYNN(r, mod, elm) \
    bp::scope().attr(BOOST_PP_STRINGIZE(elm)) = mod.attr(BOOST_PP_STRINGIZE(elm)); \
    bp::object(bp::scope().attr(BOOST_PP_STRINGIZE(elm))).attr("__module__") = "pyhmf";

void _loadPyNNErrors()
{
	bp::object pynnErrors = bp::import("pyNN.errors");

	// Copy errors from PyNN and rename their __module__ attribute using ERROR_SEQ
    BOOST_PP_SEQ_FOR_EACH(                                                      \
        COPY_FROM_PYNN,                                                         \
        pynnErrors,                                                                      \
        ERROR_SEQ                                                               \
    )
}

const char* PyHMFException::what() const throw()
{
    return this->message.c_str();
}

PyHMFException::PyHMFException(std::string message)
{
    this->message = message;
}

PyHMFException::~PyHMFException() throw()
{
}

// This method specifies the name of the Python exception to enable us grabbing
// the PyObject directly from pyNN.errors
const char* PyHMFException::name() const throw()
{
    return "PyHMFException";
}

// This method gets called on runtime in order to translate the C++ exception to
// a Python one. In this default implementation we import the Python exception
// from PyNN and register it as the error state using the Python C API.
void PyHMFException::translate() const
{
    bp::object pynnErrors = bp::import("pyNN.errors");
    bp::object error_t = pynnErrors.attr(name());
    bp::object instance = error_t(what());
    Py_INCREF(error_t.ptr());
    Py_INCREF(instance.ptr());
    PyErr_SetObject(error_t.ptr(), instance.ptr());
}


PyIndexError::PyIndexError(std::string message) : PyHMFException(message) {}
const char* PyIndexError::name() const throw() { return "IndexError"; }
void PyIndexError::translate() const
{
    PyErr_SetString(PyExc_IndexError, what());
}



// The following exceptions are very simple subclasses of PyHMFException. They
// just wrap their specific Python exceptions and pass a message to them.
PyInvalidParameterValueError::PyInvalidParameterValueError(std::string message) : PyHMFException(message) {}
const char* PyInvalidParameterValueError::name() const throw() { return "InvalidParameterValueError"; }

PyInvalidDimensionsError::PyInvalidDimensionsError(std::string message) : PyHMFException(message) {}
const char* PyInvalidDimensionsError::name() const throw() { return "InvalidDimensionsError"; }

PyConnectionError::PyConnectionError(std::string message) : PyHMFException(message) {}
const char* PyConnectionError::name() const throw() { return "ConnectionError"; }

PyInvalidModelError::PyInvalidModelError(std::string message) : PyHMFException(message) {}
const char* PyInvalidModelError::name() const throw() { return "InvalidModelError"; }

PyNoModelAvailableError::PyNoModelAvailableError(std::string message) : PyHMFException(message) {}
const char* PyNoModelAvailableError::name() const throw() { return "NoModelAvailableError"; }

PyNothingToWriteError::PyNothingToWriteError(std::string message) : PyHMFException(message) {}
const char* PyNothingToWriteError::name() const throw() { return "NothingToWriteError"; }

PyInvalidWeightError::PyInvalidWeightError(std::string message) : PyHMFException(message) {}
const char* PyInvalidWeightError::name() const throw() { return "InvalidWeightError"; }

PyNotLocalError::PyNotLocalError(std::string message) : PyHMFException(message) {}
const char* PyNotLocalError::name() const throw() { return "NotLocalError"; }

// NonExistentParameterError is a more sofisticated exception with an own
// translation mechanism.
PyNonExistentParameterError::PyNonExistentParameterError(std::string parameter_name,
            std::string model_name,
            std::vector<std::string> valid_parameter_names) : PyHMFException("")
{
    this->parameter_name = parameter_name;
    this->model_name = model_name;
    this->valid_parameter_names = valid_parameter_names;
    // TODO: Sort valid_parameter_names
}

PyNonExistentParameterError::~PyNonExistentParameterError() throw() {} 

void PyNonExistentParameterError::translate() const
{
    bp::object pynnErrors = bp::import("pyNN.errors");
    bp::object error_t = pynnErrors.attr(name());
    bp::object instance = error_t(parameter_name,
        model_name,
        valid_parameter_names);
    Py_INCREF(error_t.ptr());
    Py_INCREF(instance.ptr());
    PyErr_SetObject(error_t.ptr(), instance.ptr());
}

const char* PyNonExistentParameterError::name() const throw()
{
    return "NonExistentParameterError";
}

// RecordingError is a more complex exception, too.
PyRecordingError::PyRecordingError(std::string variable, euter::CellType cell_type) : PyHMFException("")
{
    this->variable = variable;
    this->cell_type = cell_type;
}

PyRecordingError::PyRecordingError() : PyHMFException::PyHMFException("")
{
}

PyRecordingError::~PyRecordingError() throw()
{
}

void PyRecordingError::translate() const
{
    // FIXME: This is really ugly. Should implement a reverse lookup in py_models!
    //bp::object pynnCelltypes = bp::import("pyNN.standardmodels.cells");
    //bp::object ct = bp::object(pynnCelltypes.attr(getCellTypeName(cell_type).c_str()));
    
    bp::object ct = resolveCellType(cell_type);
    
    bp::object pynnErrors = bp::import("pyNN.errors");
    bp::object error_t = pynnErrors.attr(name());
    
    //bp::dict params = bp::extract<bp::dict>(ct.attr("default_parameters"));
    // TODO: cell parameters?
    bp::object instance = error_t(variable, ct);
    Py_INCREF(error_t.ptr());
    Py_INCREF(instance.ptr());
    PyErr_SetObject(error_t.ptr(), instance.ptr());
}

const char* PyRecordingError::name() const throw()
{
    return "RecordingError";
}
