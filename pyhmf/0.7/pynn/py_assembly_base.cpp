#include "py_assembly_base.h"
#include "py_id.h"

#include "pyhmf/boost_python.h"
#include "pycellparameters/pyparameteraccess.h"

#include "euter/exceptions.h"
#include "euter/population_view.h"
#include <boost/make_shared.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

using namespace euter;

PyAssemblyBase::~PyAssemblyBase()
{
}

// TODO remove after a reasonable amount of functions is implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

/// Return a 2-column numpy array containing cell ids and spike times for
/// recorded cells.
/// Useful for small populations, for example for single neuron Monte-Carlo.
bp::object PyAssemblyBase::getSpikes(bool gather, bool compatible_output) const
{
	size_t num_spikes = 0;
	apply([&num_spikes](PopulationView const& view) {
		Population const& pop = view.population();
		for (size_t ii = 0; ii < pop.size(); ++ii) {
			if (!view.mask()[ii]) {
				continue;
			}
			num_spikes += pop.getSpikes(ii).size();
		}
	});

	// TODO: we do not need the extra shared_ptr layer as pyublas already has handler semantics
	auto matrix = boost::make_shared<py_matrix_type>(num_spikes, 2);

	auto& data = matrix->as_ublas();

	size_t pos = 0;
	size_t id_offset = 0;
	apply([&pos, &id_offset, &data](PopulationView const& view) {
		Population const& pop = view.population();
		for (size_t ii = 0; ii < pop.size(); ++ii) {
			if (!view.mask()[ii]) {
				continue;
			}

			for (auto const& time : pop.getSpikes(ii)) {
				// In Populations or PopulationViews, the cell id is the
				// index within the Population, cf. issue #1955
				// In Assemblies, there is an offset given by the sum of
				// the Population sizes of the previous items (Population
				// or PopulationView).
				data(pos, 0) = ii + id_offset;
				// in milliseconds, cf. other PyNN implementations and issue #1955
				data(pos, 1) = time * 1e3;
				++pos;
			}
		}
		id_offset += pop.size(); // add population size to offset
	});

	// this should be ok, and doesn't require internal synchronization.
	// The operator= is thread sage according to the boost docs.
	const_cast<boost::shared_ptr<py_matrix_type>&>(mSpikes) = matrix;

	return bp::object(mSpikes->to_python());
}


/// Return a 3-column numpy array containing cell ids and synaptic
/// conductances for recorded cells.
py_vector_type PyAssemblyBase::get_gsyn(bool gather, bool compatible_output)
{
	NOT_IMPLEMENTED();
}


/// Returns the number of spikes for each neuron.
py_vector_type PyAssemblyBase::get_spike_counts(bool gather)
{
	py_vector_type counts;
	auto collect_counts = [](PopulationView const& view, py_vector_type& counts) {
		size_t const pos = counts.size();
		counts.resize(pos+view.size());

		Population const& pop = view.population();
		size_t i_c=0;
		for (size_t ii=0; ii<pop.size(); ++ii) {
			if (view.mask()[ii]) {
				counts[pos+i_c] = view.population().getSpikes(ii).size();
				++i_c;
			}
		}
	};
	apply(std::bind(collect_counts, std::placeholders::_1, std::ref(counts)));

	return counts;
}


/// Return a 2-column numpy array containing cell ids and Vm for
/// recorded cells.
bp::object
PyAssemblyBase::get_v(bool gather, bool compatible_output)
{
	auto collect_membrane_voltages = [](PopulationView const& view,
	                                    py_matrix_type& data, size_t& id_offset) {
		Population const& pop = view.population();
		for (size_t ii = 0; ii < pop.size(); ii++) {
			if (!view.mask()[ii])
				continue;

			size_t pos = data.size1();
			auto const& voltageTrace = view.population().getMembraneVoltageTrace(ii);
			// append to data → enlarge and use pos as index offset below
			data.resize(pos + voltageTrace.size(), 3);

			for (auto const& tv_pair : voltageTrace) {
				// In Populations or PopulationViews, the cell id is the
				// index within the Population, cf. issue #1955
				// In Assemblies, there is an offset given by the sum of
				// the Population sizes of the previous items (Population
				// or PopulationView).
				data(pos, 0) = ii + id_offset;
				// in milliseconds, cf. other PyNN implementations and issue #1955
				data(pos, 1) = std::get<0/*time*/>(tv_pair);
				data(pos, 2) = std::get<1/*value*/>(tv_pair);
				++pos;
			}
		}
		id_offset += pop.size(); // add population size to offset
	};

	auto data = boost::make_shared<py_matrix_type>(0, 3);
	size_t id_offset = 0; // id_offset needed for assemblies
	apply(std::bind(collect_membrane_voltages, std::placeholders::_1, std::ref(*data),
					id_offset));

	// this should be ok, and doesn't require internal synchronization.
	// The operator= is thread sage according to the boost docs.
	const_cast<boost::shared_ptr<py_matrix_type>&>(mVoltageTraces) = data;

	return bp::object(mVoltageTraces->to_python());
}


/// Given the ID(s) of cell(s) in the PyPopulation, return its (their) index
/// (order in the PyPopulation).
/// >>> assert p.id_to_index(p[5]) == 5
/// >>> assert p.id_to_index(p.index([1,2,3])) == [1,2,3]
size_t PyAssemblyBase::id_to_index(PyID id)
{
	NOT_IMPLEMENTED();
}

std::vector<size_t> PyAssemblyBase::id_to_index(std::vector<PyID> ids)
{
	NOT_IMPLEMENTED();
}

npyarray PyAssemblyBase::id_to_index(npyarray ids)
{
	NOT_IMPLEMENTED();
}

/// Set initial values of state variables, e.g. the membrane potential.
/// `value` may either be a numeric value (all neurons set to the same
/// value) or a `RandomDistribution` object (each neuron gets a
/// different value)
void PyAssemblyBase::initialize(Parameter variable, double value)
{
	NOT_IMPLEMENTED();
}

void PyAssemblyBase::initialize(Parameter variable, PyRandomDistribution rnd)
{
	NOT_IMPLEMENTED();
}

/// Returns the mean number of spikes per neuron.
double PyAssemblyBase::meanSpikeCount(bool gather) const
{
	getSpikes();

	double latest = 0;
	auto first = mSpikes->begin1();
	for (size_t ii=0; ii<mSpikes->size1(); ++ii) {
		if ((*mSpikes)(ii, 1) > latest) {
			latest = (*mSpikes)(ii, 1);
		}
	}
	return mSpikes->size1()/latest /*ms*/ * 1000;
}

/// Write spike times to file.
/// file should be either a filename or a PyNN File object.
/// If compatible_output is True, the format is "spiketime cell_id",
/// where cell_id is the index of the cell counting along rows and down
/// columns (and the extension of that for 3-D).
/// This allows easy plotting of a `raster' plot of spiketimes, with one
/// line for each cell.
/// The timestep, first id, last id, and number of data points per cell are
/// written in a header, indicated by a '#' at the beginning of the line.
/// If compatible_output is False, the raw format produced by the simulator
/// is used. This may be faster, since it avoids any post-processing of the
/// spike files.
/// For parallel simulators, if gather is True, all data will be gathered
/// to the master node and a single output file created there. Otherwise, a
/// file will be written on each node, containing only the cells simulated
/// on that node.
void PyAssemblyBase::printSpikes(bp::object const& file, bool gather, bool compatible_output)
{
	getSpikes();

	bp::object f;
	if(bp::extract<std::string>(file).check())
	{
		bp::object pynnFiles = bp::import("pyNN.recording.files");
		bp::object file_t = pynnFiles.attr("StandardTextFile");
		f = file_t(static_cast<std::string>(bp::extract<std::string>(file)), "w");
	}
	else
	{
		f = file;
	}

	bp::dict metadata;

	if (compatible_output) {
		// As getSpikes has collected spikes in [cell_id, spiketime] pairs, but
		// printSpikes requires the opposite order for compatible output, we
		// have to swap the spiketime and cell_id columns.
		// The swapping can be done in place, as mSpikes is filled newly
		// everytime getSpikes() is called.
		auto& ublas_matrix = mSpikes->as_ublas();
		typedef std::remove_reference<decltype(ublas_matrix)>::type ublas_matrix_type;
		boost::numeric::ublas::matrix_column<ublas_matrix_type> col0(ublas_matrix, 0);
		boost::numeric::ublas::matrix_column<ublas_matrix_type> col1(ublas_matrix, 1);
		col0.swap(col1);
	}
	f.attr("write")(*mSpikes, metadata);
}

// printSpikes(file, gather = True, compatible_output = True); // TODO

/// Write synaptic conductance traces to file.
/// file should be either a filename or a PyNN File object.
/// If compatible_output is True, the format is "t g cell_id",
/// where cell_id is the index of the cell counting along rows and down
/// columns (and the extension of that for 3-D).
/// The timestep, first id, last id, and number of data points per cell are
/// written in a header, indicated by a '#' at the beginning of the line.
/// If compatible_output is False, the raw format produced by the simulator
/// is used. This may be faster, since it avoids any post-processing of the
/// voltage files.
void PyAssemblyBase::print_gsyn(path const& file, bool gather, bool compatible_output)
{

}

// print_gsyn(file, gather = True, compatible_output = True); // TODO

/// Write membrane potential traces to file.
/// file should be either a filename or a PyNN File object.
/// If compatible_output is True, the format is "v cell_id",
/// where cell_id is the index of the cell counting along rows and down
/// columns (and the extension of that for 3-D).
/// The timestep, first id, last id, and number of data points per cell are
/// written in a header, indicated by a '#' at the beginning of the line.
/// If compatible_output is False, the raw format produced by the simulator
/// is used. This may be faster, since it avoids any post-processing of the
/// voltage files.
/// For parallel simulators, if gather is True, all data will be gathered
/// to the master node and a single output file created there. Otherwise, a
/// file will be written on each node, containing only the cells simulated
/// on that node.
void PyAssemblyBase::print_v(path const& file, bool gather, bool compatible_output)
{

}
// print_v(file, gather = True, compatible_output = True); // TODO


void PyAssemblyBase::set_record(std::string parameter_name, bool value)
{
	apply([parameter_name, value](PopulationView & p) {
			std::vector<ParameterProxy> proxy = getPyParameterVector(p);
			for(size_t ii = 0; ii < proxy.size(); ++ii)
			{
				proxy[ii].set(parameter_name, bp::object(value));
			}
		});
}

/// Record spikes from all cells in the PyPopulation.
void PyAssemblyBase::record(bool to_file)
{
	set_record("record_spikes", true);
}


/// Record synaptic conductances for all cells in the PyPopulation.
void PyAssemblyBase::record_gsyn(bool to_file)
{
	set_record("record_gsyn", true);
}


/// Record the membrane potential for all cells in the PyPopulation.
void PyAssemblyBase::record_v(bool)
{
	set_record("record_v", true);
}

/// Save positions to file. The output format is id x y z
void PyAssemblyBase::save_positions(path const& file)
{
	NOT_IMPLEMENTED();
}
