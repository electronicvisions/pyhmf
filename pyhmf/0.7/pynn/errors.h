#pragma once

#include <string>
#include <vector>
#include "pyhmf/boost_python_fwd.h"
#include <Python.h>

#ifndef PYPLUSPLUS
#include "euter/celltypes.h"
#endif

void _loadPyNNErrors();

class PyHMFException: public std::exception
{
public:
    PyHMFException(std::string message);
    ~PyHMFException() throw();
    virtual const char* what() const throw();
    virtual const char* name() const throw();
    virtual void translate() const;
private:
    std::string message;
};

class PyIndexError : public PyHMFException
{
public:
	PyIndexError(std::string message);
	const char* name() const throw();
	void translate() const;
};

class PyInvalidParameterValueError: public PyHMFException
{
public:
    PyInvalidParameterValueError(std::string message);
    virtual const char* name() const throw();
};

class PyNonExistentParameterError: public PyHMFException
{
public:
    PyNonExistentParameterError(std::string parameter_name, std::string model_name, std::vector<std::string> valid_parameter_names);
    ~PyNonExistentParameterError() throw();
    void translate() const;
    virtual const char* name() const throw();
private:
    std::string parameter_name;
    std::string model_name;
    std::vector<std::string> valid_parameter_names;
};

class PyInvalidDimensionsError: public PyHMFException
{
public:
    PyInvalidDimensionsError(std::string message);
    virtual const char* name() const throw();
};

class PyConnectionError: public PyHMFException
{
public:
    PyConnectionError(std::string message);
    virtual const char* name() const throw();
};

class PyInvalidModelError: public PyHMFException
{
public:
    PyInvalidModelError(std::string message);
    virtual const char* name() const throw();
};

class PyNoModelAvailableError: public PyHMFException
{
public:
    PyNoModelAvailableError(std::string message);
    virtual const char* name() const throw();
};

class PyNothingToWriteError: public PyHMFException
{
public:
    PyNothingToWriteError(std::string message);
    virtual const char* name() const throw();
};

class PyInvalidWeightError: public PyHMFException
{
public:
    PyInvalidWeightError(std::string message);
    virtual const char* name() const throw();
};

class PyNotLocalError: public PyHMFException
{
public:
    PyNotLocalError(std::string message);
    virtual const char* name() const throw();
};

class PyRecordingError: public PyHMFException
{
public:
#ifndef PYPLUSPLUS
    PyRecordingError(std::string variable, CellType cell_type);
#endif
    PyRecordingError();
    ~PyRecordingError() throw();
    virtual const char* name() const throw();
    virtual void translate() const;
private:
    std::string variable;
#ifndef PYPLUSPLUS
    CellType cell_type;
#endif
};
