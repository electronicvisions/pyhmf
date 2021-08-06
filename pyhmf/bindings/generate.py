#!/usr/bin/env python
import sys,os
import re
import logging
import argparse

DEBUG=False

from pywrap.wrapper import Wrapper
from pywrap import containers, namespaces, matchers, classes
from pywrap.namespace_util import NamespaceUtil
from pyplusplus.module_builder import call_policies

from pywrap.matchers import access_type_matcher_t, custom_matcher_t
matcher = matchers


wrap = Wrapper()
mb = wrap.mb
logging.basicConfig(level=logging.INFO)

#if not DEBUG:
#    # mute md5 and os.popen4 deprecation warnings + some UserWarning
#    import warnings
#    warnings.filterwarnings("ignore", category=DeprecationWarning)
#    warnings.filterwarnings("ignore", category=UserWarning)

pynn_classes = '''\
    Population PopulationView Assembly Projection \
    Space Structure Line Grid2D Grid3D RandomStructure Cuboid Sphere \
    AllToAllConnector \
    OneToOneConnector \
    FixedProbabilityConnector \
    DistanceDependentProbabilityConnector FixedNumberPreConnector \
    FixedNumberPostConnector FromListConnector FromFileConnector SmallWorldConnector \
    StandardCellType \
    IF_cond_exp EIF_cond_exp_isfa_ista SpikeSourcePoisson SpikeSourceArray \
    SynapseDynamics STDPMechanism TsodyksMarkramMechanism AdditiveWeightDependence \
    MultiplicativeWeightDependence AdditivePotentiationMultiplicativeDepression \
    GutigWeightDependence SpikePairRule \
    DCSource StepCurrentSource ACSource NoisyCurrentSource \
    BaseFile StandardTextFile PickleFile NumpyBinaryFile HDF5ArrayFile \
    AbstractRNG NumpyRNG GSLRNG NativeRNG RandomDistribution \
    Timer ProgressBar
    '''.split()
pynn_base_classes = '''\
        AssemblyBase PopulationBase
        '''.split()
pynn_exceptions = '''\
    InvalidParameterValueError NonExistentParameterError \
    InvalidDimensionsError ConnectionError InvalidModelError NoModelAvailableError \
    RoundingWarning NothingToWriteError InvalidWeightError NotLocalError \
    RecordingError PyHMFException IndexError
    '''.split()
pynn_functions = '''\
    setup end run reset \
    get_time_step get_current_time get_min_delay get_max_delay \
    rank num_processes \
    create connect set initialize record record_v record_gsyn \
    colour notify get_script_args init_logging \
    dumpAsXml dumpAsBinary \
    '''.split()
testing_functions = '''\
    numpyExample getObjectStoreSize\
    '''.split()
shared_ptr_class_factories = '''\
    Projection \
    AllToAllConnector OneToOneConnector FixedProbabilityConnector \
    FixedNumberPreConnector FixedNumberPostConnector DistanceDependentProbabilityConnector \
    FromListConnector \
    '''.split()
base_classes = '''\
    PopulationBase AbstractRNG
    '''.split()

#'PyNNCommand CommandStore Run'.split()
helper_classes = ['RNG', 'AssemblyBase', 'PopulationBase', 'CellIterator']
helper_functions = []

# New style bindings, generate bindings for all classes starting with Py
# If a class provides a (static) function create, this instead of the
# constructor is used

py_member_filter_re = re.compile(r"(^_[^_])|(.*Cpp$)|(^impl$)")
py_member_filter = custom_matcher_t(lambda decl: py_member_filter_re.match(decl.name))

# Helper to find Py... classes
def findClass(name):
    c = None
    try:
        c = mb.class_(name)
    except matcher.declaration_not_found_t:
        pass

    # exclude XYZ if PyXYZ exists + rename it
    try:
        cc = mb.class_('Py'+name)
        if not c is None:
            c.exclude()
            logging.info("Excluding: %s", c.name)
        logging.info("Renaming %s -> %s", cc.name, name)
        cc.rename(name)
        c = cc
    except matcher.declaration_not_found_t:
        pass

    return c

missing_classes = []
missing_class_factories = []
for cl in pynn_classes + helper_classes:
    c = findClass(cl)
    if c is None:
        continue
    logging.info("Add class %s ", c.name)

    c.include()
    c.include_files.append( 'pyhmf/boost_python.h' )
    c.mem_opers(allow_empty=True).include() # all operators :)
    c.mem_opers("operator->", allow_empty=True).exclude() # Avoid warnings

    # Exclude members and functions starting with single underscore or ending with Cpp
    c.mem_funs(py_member_filter, allow_empty=True).exclude()
    c.variables(py_member_filter, allow_empty=True).exclude()

    # Exclude protected and private member functions
    c.mem_funs(access_type_matcher_t('protected'), allow_empty=True).exclude()
    c.mem_funs(access_type_matcher_t('private'), allow_empty=True).exclude()

    # some classes are exposed (in global namespace) via boost::shared_ptr :)
    if cl in shared_ptr_class_factories:
        logging.info("Add factory for " + c.name)
        createFactory = c.mem_funs('create', allow_empty=False)
        c.add_fake_constructors( createFactory )
        # bogus warning? registration seems to work!
        # mb.class_('::boost::shared_ptr<%s>' % c.name).disable_messages(messages.W1040)
        c.constructors().exclude() # disable normal constructors! shared classes!

    if cl in base_classes:
        logging.info("Disable constructors for " + c.name)
        c.constructors().exclude() # disable normal constructor!

    try:
        raw_ctor = c.mem_fun("_raw_constructor")
        c.add_declaration_code('#include "pyhmf/boost_python_raw_constructor.h"')
        c.add_registration_code('def("__init__", bp::raw_constructor(&{0}::_raw_constructor, 0))'.format(c.name))
    except RuntimeError: pass

    if cl in ['PopulationBase', 'Assembly']:
        c.add_registration_code('def("__iter__", bp::range(&Py{0}::begin, &Py{0}::end))'.format(cl))
        c.add_registration_code('def("all", bp::range(&Py{0}::begin, &Py{0}::end))'.format(cl))

    if cl == 'Projection':
        c.add_property('pre', c.mem_fun('getPre'))
        c.add_property('post', c.mem_fun('getPost'))

    # provide size attribute/property -- upon user request (pyNN.{nest,neuron,brian} do it...)
    if cl in ['Assembly', 'Projection']:
        c.add_property('size', c.mem_fun(lambda decl: decl.name == 'size' and decl.arguments == []))
    if cl in ['Assembly']: # FIXME: add impl of this for Projection too
        c.add_property('label', c.mem_fun(lambda decl: decl.name == 'label' and decl.arguments == []))

    # FIXME: (ECM) query inherited size() directly
    if cl in ['Population', 'PopulationView']:
        base = findClass('PopulationBase')
        for fn in ['size', 'label', 'celltype']:
            c.add_property(fn, base.mem_fun(fn))

    # provide size(void) -> __len__ operator
    try:
        f_len = c.mem_funs(lambda decl: decl.name == 'size' and decl.arguments == [])
        f_len.rename('__len__')
    except RuntimeError: pass

    # redirect python's __str__ to operator<<
    if cl in 'Population PopulationView Assembly Projection Run End':
        c.add_registration_code('def(bp::self_ns::str(bp::self_ns::self))')

    #for ctor in c.constructors():
    #    if ctor.is_trivial_constructor:
    #        ctor.exclude()

    ############################################################################
    # The next code section workarounds the automatic wrapping of T* members   #
    # into new classname_wrapper classes.  This is needed due to RTTI typeid() #
    # checks of boost::serialization that fail if we use a wrapper class that  #
    # derives from the real class that has to be serialized.                   #
    ############################################################################

    """
    # warn if public variables that are pointers to other classes exist
    for var in c.vars(access_type_matcher_t('public'), allow_empty=True):
        if isinstance(var.type, declarations.cpptypes.pointer_t):
            print "Cannot export %s::%s via wrapper class." % (c.name, var.name)
            print "\ttypeid(%s) would be changed to typeid(%s_wrapper)" % (c.name, c.name)
            print "\tPlease provide getter/setter functions to private variable. (boost::serialization)"
            var.exclude()
            """

mb.class_('PyID').mem_fun('id').rename('__int__')

mb.enums().include() # TODO: shouldn't be necessary?

# add support for Assembly(list(PyPopulationView, ...), kwargs)
classes.add_from_pyiterable_converter_to(mb.class_('::std::vector<PyPopulationView>'))

# add support for tset(string parametername, sequence)
classes.add_from_pyiterable_converter_to(mb.class_('::std::vector<double>'))

# Exclude typedefs:
for td in mb.global_ns.typedefs():
    if td.name in ['PyProjectionPtr', 'ProjectionPtr', 'PyConnectorPtr', 'path']:
        td.exclude()

for e in pynn_exceptions:
    c = findClass(e)
    if c:
        translate_code = """
            exc.translate();
            """
        c.exception_translation_code = translate_code


missing_functions = []
for fu in pynn_functions + helper_functions + testing_functions:
    try:
        f = mb.free_fun(fu)
    except matcher.declaration_not_found_t:
        missing_functions.append(fu)
        continue
    f.include()

# Fix pyNN setup() call
mb.add_declaration_code('#include "boost/python/raw_function.hpp"')
mb.free_fun("setup").exclude()
# ECM: bp::def(..., raw_function) does not support additional doc strings :(
#      (=> code path in bp (for raw_function) differs from other wrappings)
#      * mb.free_fun("setup").documentation = "Does not because of the raw_function wrapping";
#      * adding to initialize() would be ugly as we want to generate it automatically in the future
#      => monkey patch the wrapped function
mb.add_registration_code('bp::def("setup", bp::raw_function(setup, 0) /* , "docstring not possible" */ );')
setup_doc = """Should be called at the very beginning of a script.

extra_params contains any keyword arguments that are required by a given simulator but not by others.

Kwargs:
    timestep:  (float) ignored
    min_delay: (float) ignored
    max_delay: (float) ignored
    marocco:   (pymarocco.PyMarocco) High-level interface to the marocco framework. Allows to set parameters of the placement, routing and parameter transformation algorithms. Provides access to post-mapping data like the mapping lookup table or mapping statistics.

Returns:
    1 (integer) number of MPI processes of the PyNN backend (always 1 for PyHMF frontend)
""".replace("\n", r"\n")
mb.add_registration_code('bp::scope().attr("setup").attr("__doc__") = bp::str("%s");' % setup_doc)

# Better something like this?
#def make_raw_function(name):
#    f = mb.free_fun(name)
#    code_gen = code_creators.custom_text_t(
#            'bp::def("{name}", bp::raw_function({name}, 0) );'.format(name=name))
#    f.adopt_creator(code_gen)
#make_raw_function("setup")


# TODO: use logging, WARN or ERROR
print("Missing class definitions (w.r.t. PyNN):")
print('\t' + ' '.join(missing_classes))
if missing_class_factories:
    print("Missing class factories:")
    print('\t' + ' '.join(missing_class_factories))
print("Missing function definitions (w.r.t. PyNN):")
print('\t' + ' '.join(missing_functions))

### END

mb.add_declaration_code('#include "pyhmf/boost_python.h"')
mb.add_registration_code('bp::import("pyublas");', False)
mb.add_registration_code(
    '''
    initialize();
    ''' )

# expose only public interfaces
namespaces.exclude_by_access_type(mb, ['variables', 'calldefs', 'classes'], 'private')
namespaces.exclude_by_access_type(mb, ['variables', 'calldefs', 'classes'], 'protected')

# don't expose _XYZ() functions
mb.global_ns.calldefs(py_member_filter, allow_empty=True).exclude()

# done :)
wrap.set_number_of_files(-1)
wrap.finish()
