#include "free_functions.h"

#include <limits>
#include <fstream>

#include <boost/python/stl_iterator.hpp>
#include "pyhmf/boost_python.h"
#include "pyhmf/objectstore.h"
#include "euter/exceptions.h"
#include "euter/metadata.h"

#include "marocco/mapping.h"

using namespace euter;

double get_time_step() {
    return getStore().getTimestep();
}

double get_current_time() {
	NOT_IMPLEMENTED();
	return std::numeric_limits<double>::quiet_NaN();
}

double get_min_delay() {
    return getStore().getMinDelay();
}

double get_max_delay() {
    return getStore().getMaxDelay();
}

/// Get the MPI process rank (MPI_Comm_rank); low priority!
int rank() {
	return 0;
}

/// Get the number of MPI processes (MPI_Comm_size); low priority!
int num_processes() {
	return 1;
}

int clear()
{
	resetStore();
	return 0;
}

// special user-side implementations
int run(double runtime)
{
	auto store = getStore();

	store.run(runtime);

	marocco::mapping::run(store);

	// check the result
	store.check();

	return 1;
}


// we have to override this in python <= extra arguments
int setup(bp::tuple args,
          bp::dict kwparam)
{
	if (len(args) > 0)
	{
		throw std::runtime_error("Setup accepts only keyword arguments");
	}
	ObjectStore::Settings settings;

	settings.timestep  = bp::extract<double>(kwparam.get("timestep",0.1));
	settings.min_delay = bp::extract<double>(kwparam.get("min_delay",0.1));
	settings.max_delay = bp::extract<double>(kwparam.get("max_delay",10.0));

	ObjectStore::metadata_map metadata;

	for (bp::stl_input_iterator<bp::tuple> it(kwparam.iteritems()), end; it != end; it++)
	{
		bp::tuple key_value = *it;
		// ECM (2016-09-01): We pass the shared_ptr  to C++, so let's
		// increase the python reference counter; otherwise Python will clean up
		// on exit and boost python will clear the shared_ptr too => SEGFAULT on
		// exit.
		bp::incref(bp::object(key_value[1]).ptr());

		std::string key = bp::extract<std::string>(key_value[0]);
		bp::extract< boost::shared_ptr<MetaData> > metadata_obj(key_value[1]);
		if(metadata_obj.check())
		{
			boost::shared_ptr<MetaData> m = metadata_obj();
			if (m->name() == key)
			{
				metadata[key] = m;
			}
			else
			{
				std::stringstream error;
				error << "Keyword '" << key
				      << "' has non matching MetaData object '"
				      << m->name() << "'";
				throw std::runtime_error(error.str());
			}
		}
	}

	resetStore(); // clear all data
	getStore().setup(settings, metadata);
	return 1;
}

int reset()
{
	getStore().reset();
	return 1;
}

int end(bool) {
	// The PyNN (0.7) API suggests to flush to disk in this function while
	// executing the simulation in run().
	// We do not support this yet.
	// Due to the early writeout in run() the file format is also not
	// configurable (but the lower layers do not support this either).
	resetStore();
	return 1;
}

void dumpAsXml(std::string filename)
{
	std::ofstream out(filename);
	boost::archive::xml_oarchive ar(out);
	ar << boost::serialization::make_nvp("object", getStore());
}

void dumpAsBinary(std::string filename)
{
	std::ofstream out(filename);
	boost::archive::binary_oarchive ar(out);
	ar << boost::serialization::make_nvp("object", getStore());
}

