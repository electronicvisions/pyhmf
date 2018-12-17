#include "py_models.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include "pyhmf/boost_python.h"
#include "euter/celltypes.h"
#include "euter/exceptions.h"

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

// Expands to (for elm being IF_brainscales_hardware):
// bp::scope().attr("IF_brainscales_hardware") = pynnCelltypes.attr("IF_brainscales_hardware");
// bp::object(bp::scope().attr("IF_brainscales_hardware")).attr("__module__") = "pyhmf";
#define COPY_FROM_PYNN(r, mod, elm) \
    bp::scope().attr(BOOST_PP_STRINGIZE(elm)) = mod.attr(BOOST_PP_STRINGIZE(elm)); \
    bp::object(bp::scope().attr(BOOST_PP_STRINGIZE(elm))).attr("__module__") = "pyhmf";

// Expands to (for elm being IF_brainscales_hardware):
// (CellType::IF_brainscales_hardware, bp::scope().attr("IF_brainscales_hardware"))
#define BUILD_RELATION(r, _, elm) \
    (euter::CellType::elm, bp::scope().attr(BOOST_PP_STRINGIZE(elm)))

typedef boost::bimap<euter::CellType, bp::object> ct_map_t;
const ct_map_t celltypeMap;

// Shame on me... TODO CK place later in a better position
static const char custom_module_init_code[] =
"def make_EIF_multicond_exp_isfa_ista_defaults():\n"
"    import pycellparameters\n"
"    model = pycellparameters.EIF_multicond_exp_isfa_ista()\n"
"    return dict((n, getattr(model, n)) for n in model.getNames())\n"
"\n"
"def make_IF_multicond_exp_defaults():\n"
"    import pycellparameters\n"
"    model = pycellparameters.IF_multicond_exp()\n"
"    return dict((n, getattr(model, n)) for n in model.getNames())\n"
"\n"
"class IF_brainscales_hardware(StandardCellType):\n"
"    \"\"\"\n"
"    Leaky integrate and fire model with conductance-based synapses and fixed\n"
"    threshold as it is resembled by the FACETS Hardware Stage 1.\n"
"    \n"
"    The following parameters can be assumed for a corresponding software\n"
"    simulation: cm = 0.2 nF, tau_refrac = 1.0 ms, e_rev_E = 0.0 mV.\n"
"    For further details regarding the hardware model see the FACETS-internal Wiki:\n"
"    https://facets.kip.uni-heidelberg.de/private/wiki/index.php/WP7_NNM\n"
"    \"\"\""
"\n"
"    default_parameters = EIF_cond_exp_isfa_ista.default_parameters\n"
"    recordable = ['spikes', 'v']\n"
"    default_initial_values = {\n"
"        'v': -65.0, #'v_rest',\n"
"    }\n"
"\n"
"class EIF_multicond_exp_isfa_ista(StandardCellType):\n"
"    \"\"\"\n"
"    As EIF_cond_exp_isfa_ista, but with support for multiple conductances.\n"
"    \"\"\"\n"
"    default_initial_values = {}\n"
"    default_parameters = make_EIF_multicond_exp_isfa_ista_defaults()\n"
"    recordable = ['v']\n"
"\n"
"class IF_multicond_exp(StandardCellType):\n"
"    \"\"\"\n"
"    As IF_cond_exp, but with support for multiple conductances.\n"
"    \"\"\"\n"
"    default_initial_values = {}\n"
"    default_parameters = make_IF_multicond_exp_defaults()\n"
"    recordable = ['v']\n"
"\n";

void _loadPyNNCellTypes()
{
	bp::object pynnCelltypes = bp::import("pyNN.standardmodels.cells");

	// Monkey patch class IF_brainscales_hardware in pyNN.standardmodels.cells :)
	bp::exec(custom_module_init_code, pynnCelltypes.attr("__dict__"));

	// Copy celltypes from PyNN and rename their __module__ attribute using
	// CELL_TYPE_SEQ defined in euter/celltypes.h
    BOOST_PP_SEQ_FOR_EACH(                                                      \
        COPY_FROM_PYNN,                                                         \
        pynnCelltypes,                                                          \
        CELL_TYPE_SEQ                                                           \
    )

    // Build conversion map.
	const_cast<ct_map_t&>(celltypeMap) = boost::assign::list_of<ct_map_t::relation>
        BOOST_PP_SEQ_FOR_EACH(                                                  \
            BUILD_RELATION,                                                     \
            _,                                                                  \
            CELL_TYPE_SEQ                                                       \
        )
        ;
}

bp::object resolveCellType(euter::CellType celltype)
{
	if(celltypeMap.right.empty())
	{
		euter::InvalidParameter("No Neuron Types registered :(");
	}

	auto it = celltypeMap.left.find(celltype);
	if(it == celltypeMap.left.end())
	{
		euter::InvalidParameter("Invalid/Unkown Neuron Type");
	}
	return it->second;
}

euter::CellType resolveCellType(bp::object obj)
{
	if(obj.is_none())
	{
		euter::InvalidParameter("None Neuron Type");
	}

	if(celltypeMap.right.empty())
	{
		euter::InvalidParameter("No Neuron Types registered :(");
	}
	
	auto it = celltypeMap.right.find(obj);
	if(it == celltypeMap.right.end())
	{
		euter::InvalidParameter("Invalid/Unkown Neuron Type");
	}
	return it->second;
}
