#pragma once

#include "pyhmf/boost_python_fwd.h"

// We pull in the default objects from PyNN

/// Import cellTypes from pyNN
/// @note: Ensure that it is run at modules start time
void _loadPyNNCellTypes();

#ifndef PYPLUSPLUS
#include <map>
#include "euter/celltypes.h"

CellType resolveCellType(bp::object obj);
bp::object resolveCellType(CellType celltype);

#endif
