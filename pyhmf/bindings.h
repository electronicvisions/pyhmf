#include <string>
#include <map>

// GCCXML has problems with atomics -> removed before boost shared_prt is included
#ifdef PYPLUSPLUS
#undef __ATOMIC_RELAXED
#undef __ATOMIC_ACQUIRE
#undef __ATOMIC_RELEASE
#endif // PYPLUSPLUS
#include <boost/shared_ptr.hpp>

#include "pynn/api.h"

using namespace euter;

// central (overrides everything) renaming of C++ types for python wrapping
namespace pyplusplus{ namespace aliases {
	typedef std::map<std::string, double>           mapStringDouble;
	typedef std::vector<double>                     vectorDouble;
	typedef std::vector<unsigned int, unsigned int> vectorIntInt;
} } //pyplusplus::aliases

// make templated stuff visible for code generator
namespace py_details {
    inline void instantiate() {
		size_t a;
		static_cast<void>(a);

		a = sizeof( boost::shared_ptr<PyProjection> );
		a = sizeof( boost::shared_ptr<PyConnector> );
		a = sizeof( boost::shared_ptr<PyAllToAllConnector> );
		a = sizeof( boost::shared_ptr<PyOneToOneConnector> );
		a = sizeof( boost::shared_ptr<PyFixedProbabilityConnector> );
		a = sizeof( boost::shared_ptr<PyFromListConnector> );
// TODO	a = sizeof( boost::shared_ptr<PyFixedNumberPreConnector> );

		a = sizeof( std::vector<PyPopulationView> );
		a = sizeof( std::vector<double> );
    }
}
