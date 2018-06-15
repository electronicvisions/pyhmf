#pragma once

#include <iosfwd>
#include <map>
#include <boost/filesystem/path.hpp>

// Py++ compatible boost python stuff
#include "pyhmf/boost_python_fwd.h"

#include "shared_ptr_fwd.h"
#include "std_function.h"
#include "py_random.h"
#include "pyublas.h"

class PyID;
class PyPopulation;
class PyPopulationView;
class PyAssembly;
class PopulationView;

typedef bp::object Parameter;
typedef std::map<std::string, Parameter> ParameterDict;

typedef int NeuronId;

typedef boost::filesystem::path path;

class PyAssemblyBase : public SentinelKeeper
{
public:
	virtual ~PyAssemblyBase();

	/// Returns a human-readable description of the population.
	/// The output may be customized by specifying a different template
	/// togther with an associated template engine (see ``pyNN.descriptions``).
	/// If template is None, then a dictionary containing the template context
	/// will be returned.
	// TODO std::string describe(std::string tmpDemplate = population_default.txt, engine = default);
	// Implement in python ?

	/// Return a 2-column numpy array containing cell ids and spike times for
	/// recorded cells.
	/// Useful for small populations, for example for single neuron Monte-Carlo.
	bp::object getSpikes(
		bool gather = true,
		bool compatible_output = true) const;

	/// Return a 3-column numpy array containing cell ids and synaptic
	/// conductances for recorded cells.
	py_vector_type get_gsyn(bool gather = true, bool compatible_output = true);

	/// Returns the number of spikes for each neuron.
	py_vector_type get_spike_counts(bool gather = true);

	/// Return a 2-column numpy array containing cell ids and Vm for
	/// recorded cells.
	bp::object get_v(bool gather = true, bool compatible_output = true);

	/// Given the ID(s) of cell(s) in the PyPopulation, return its (their) index
	/// (order in the PyPopulation).
	/// >>> assert p.id_to_index(p[5]) == 5
	/// >>> assert p.id_to_index(p.index([1,2,3])) == [1,2,3]
	size_t id_to_index(PyID id);
	std::vector<size_t> id_to_index(std::vector<PyID> ids);
	npyarray id_to_index(npyarray ids);

	/// Set initial values of state variables, e.g. the membrane potential.
	/// `value` may either be a numeric value (all neurons set to the same
	/// value) or a `RandomDistribution` object (each neuron gets a
	/// different value)
	void initialize(Parameter variable, double value);
	void initialize(Parameter variable, PyRandomDistribution rnd);

	/// Returns the mean number of spikes per neuron.
	double meanSpikeCount(bool gather = true) const;

	/// Connect a current source to all cells in the PyPopulation.
	// inject(current_source); // TODO
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
	void printSpikes(bp::object const& file, bool gather = true, bool compatible_output = true);
	// TODO void printSpikes(PyNNFile file, bool gather = true, bool compatible_output = true);

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
	void print_gsyn(path const& file, bool gather = true, bool compatible_output = true);
	// TODO void print_gsyn(PyNNFile file, bool gather = true, bool compatible_output = true);

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
	void print_v(path const& file, bool gather = true, bool compatible_output = true);
	// TODO void print_v(PyNNFile file, bool gather = true, bool compatible_output = true);

	/// Record spikes from all cells in the PyPopulation.
	void record(bool to_file = true);

	/// Record synaptic conductances for all cells in the PyPopulation.
	void record_gsyn(bool to_file = true);

	/// Record the membrane potential for all cells in the PyPopulation.
	void record_v(bool = true);

	/// Save positions to file. The output format is id x y z
	void save_positions(path const& file);

protected:
	virtual void apply(std::function<void(PopulationView &)> f) = 0;
	virtual void apply(std::function<void(PopulationView const&)> f) const = 0;

private:
	void set_record(std::string parameter_name, bool value);

	boost::shared_ptr<py_matrix_type> mSpikes;
	boost::shared_ptr<py_matrix_type> mVoltageTraces;
};
