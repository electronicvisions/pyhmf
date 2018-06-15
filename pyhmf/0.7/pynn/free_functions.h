#pragma once

// Py++ compatible boost python stuff
#include "pyhmf/boost_python_fwd.h"

#include <string>

// we have to override this in python <= extra arguments
int setup(bp::tuple args,
          bp::dict extra_params = SentinelKeeper::emptyPyDict);

// Do any necessary cleaning up before exiting (blocks until experiment terminates).
int end(bool compatible_output = true);

// Reset the time to zero, neurons (?), weights to initial values, delete any recorded data (FIXME?).
// The network structure is not changed, nor is the specification of which neurons to record from.
int reset();

// Not implemented in hardware.
double get_time_step();

// Not implemented in hardware.
double get_current_time();

// Return the minimum allowed synaptic delay.
double get_min_delay();

// Return the maximum allowed synaptic delay.
double get_max_delay();

// Return the MPI rank of the current node. (FIXME?)
int rank();

// Return the number of MPI processes. (FIXME?)
int num_processes();

// Run the emulation for simtime ms.
int run(double simtime);

// Dumps the Object Store to a file
void dumpAsXml(std::string filename);
void dumpAsBinary(std::string filename);


// Clear all holded PyNN Obejects on the C++ Side, objects holded by python might
// be still accessible.
// @note: This ist not PyNN standart, but usefull for unittests
int clear();
