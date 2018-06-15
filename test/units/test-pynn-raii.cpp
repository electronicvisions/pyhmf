#include "pynn/api.h"
#include "pynn/boost_python.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( test_pynn_raii )

BOOST_AUTO_TEST_CASE( test_pynn_raii )
{
		setup();

		auto pop0 = PyPopulation(23, PyIF_brainscales_hardware);
		auto pop1 = PyPopulation(42, PyIF_brainscales_hardware);
		auto proj = connect(PyAssembly(pop0),
		                    PyAssembly(pop1),
		                    SentinelKeeper::emptyPyObject,
		                    SentinelKeeper::emptyPyObject,
		                    /*synapse_type*/"", /*p*/0.3);

		// dangeling pointers
		create(PyIF_cond_exp, PyPopulation::emptyPyDict, 10);
		connect(PyAssembly(pop0), PyAssembly(pop1));

		// scoped destruction
		// CK -> this might not work as expected, because a reference to the
		// underling Population is stored in the command store
		{
			PyPopulation(55, PyIF_brainscales_hardware);
		}

		// explicit delete
		// CK -> See above
		{
			PyPopulation a = PyPopulation(25, PyIF_brainscales_hardware);
		}
		run(1000);

		end();

		setup();
		// nothing?
		run(10);

		// TODO: We should check for correct command stack contents here?
}

BOOST_AUTO_TEST_SUITE_END()
