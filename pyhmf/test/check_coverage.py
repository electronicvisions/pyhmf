"""
For every function, class and method found in the common module, this script
checks that the same function, etc, with the same argument names, exists in the
simulator-specific modules.

Needs to be extended to check that arguments have the same default arguments
(see http://www.faqts.com/knowledge_base/view.phtml/aid/5666 for how to obtain
default values of args).

Andrew P. Davison, CNRS, UNIC, May 2006
$Id: checkAPI.py 812 2010-11-03 15:36:29Z apdavison $
"""

import re, string, types, getopt, sys, shutil, os, inspect, math, argparse
from lxml import etree
from console import *
from pyNN import common
import pyhmf

# Basic inspect functionality for Boost Python generated methods
def getargspec_bp(func):

    args = None
    vargs = None
    keywords = None
    defaults = None
    
    signature = re.search(r'[^\(]*\((?P<args>.*)\)', func.__doc__.split(':')[0]).group(1).strip()
    
    args = [arg[0] if arg[0] != 'arg1' else 'self' for arg in re.findall(r'[^\w](?P<arg>\w+)(,|$)', signature)]
    
    return inspect.ArgSpec(args, vargs, keywords, defaults)


# Note that we exclude built-ins, modules imported from the standard library,
# and classes defined only in common.
exclude_list = ['__module__','__doc__','__builtins__','__file__','__class__',
                '__delattr__', '__dict__', '__getattribute__', '__hash__',
                '__new__','__reduce__','__reduce_ex__','__repr__','__setattr__',
                '__str__','__weakref__',
                'time','types','copy', 'sys', 'numpy', 'random',
                '_abstract_method', '_function_id', 'distance', 'distances',
                'build_translations',
                'InvalidParameterValueError', 'NonExistentParameterError',
                'InvalidDimensionsError', 'ConnectionError', 'RoundingWarning',
                'StandardCellType', 'Connector', 'StandardModelType',
                'STDPTimingDependence', 'STDPWeightDependence', 'ShortTermPlasticityMechanism',
                'ModelNotAvailable', 'IDMixin',
                '_tcall', '_call',
                ] + dir(math)


# Helper classes for status of API tests
class Result:
    color = DEFAULT
    label = ''
    
    def __init__(self, msg=None):
        self.message = msg

    
    def to_cstring(self):
        return CString(self.label.upper(), fg=self.color, fmt=BOLD)


    def __str__(self):
        return str(self.to_cstring())


class Success(Result):

    color = GREEN
    label = 'ok'


class NotFound(Result):

    color = RED
    label = 'not found'


class InconsistentSignature(Result):

    color = YELLOW
    label = 'signature'


def funcArgs(func):
    if hasattr(func,'im_func'):
        func = func.im_func
    if hasattr(func,'func_code'):
        code = func.func_code
        fname = code.co_name
        args = inspect.getargspec(func)
    elif hasattr(func, 'func_doc'):
        fname = func.__name__
        args = getargspec_bp(func)
    else:
        args = ()
        fname = func.__name__
    try:
        func_args = "%s(%s)" % (fname, inspect.formatargspec(*args))
        return func_args
    except TypeError, e:
        print e
        print "Error with", func
        return "%s()" % fname


def checkFunction(func):
    """Checks that the functions have the same names, argument names, and
    __doc__ strings."""

    differences = ""
    common_args = funcArgs(func)
    common_doc  = func.__doc__
    if dir(pyhmf).__contains__(func.func_name):
        modfunc = getattr(pyhmf, func.func_name)
        module_args = funcArgs(modfunc)
        if common_args == module_args:
            module_doc = modfunc.__doc__
        else:
            return InconsistentSignature(common_args + " != " + module_args)
    else:
        return NotFound()
    return Success()

def checkClass(classname):
    """Checks that the classes have the same method names and the same
    __doc__ strings."""
    str = ""
    common_doc  = getattr(common, classname).__doc__
    if dir(pyhmf).__contains__(classname):
        module_doc = getattr(pyhmf,classname).__doc__
    else:
        return NotFound()
    return Success()

def checkMethod(meth,classname):
    """Checks that the methods have the same names, argument names, and
    __doc__ strings."""
    str = ""
    differences = ""
    common_args = funcArgs(meth.im_func)
    common_doc  = meth.im_func.__doc__
    if hasattr(pyhmf, classname):
        cls = getattr(pyhmf, classname)
        if hasattr(cls, meth.im_func.func_name): #dir(cls).__contains__(meth.im_func.func_name):
            modulemeth = getattr(cls, meth.im_func.func_name)
            module_args = funcArgs(modulemeth)
            module_doc  = modulemeth.im_func.__doc__

            return InconsistentSignature(common_args + " != " + module_args)
        else:
            return NotFound()
    else:
        return NotFound()

def checkStaticMethod(meth,classname):
    """Checks that the methods have the same names, argument names, and
    __doc__ strings."""
    str = ""
    common_args = funcArgs(meth)
    common_doc  = meth.__doc__
    cls = getattr(pyhmf, classname)
    if dir(cls).__contains__(meth.func_name):
        modulemeth = getattr(cls,meth.func_name)
        module_args = funcArgs(modulemeth)
        module_doc = modulemeth.__doc__
        return InconsistentSignature(common_args + "!=" + module_args)
    else:
        return NotFound()

def checkData(varname):
    """Checks that all modules contain data items with the same name."""
    str = ""
    if dir(pyhmf).__contains__(varname):
        return Success()
    else:
        return NotFound()


# XML report helper classes
class Report(list):
    
    def __init__(self, name):
    
        list.__init__(self)
        self.name = name
        
        self.tests = 0
        self.failures = 0
        self.errors = 0


    def append(self, ts):
    
        if not isinstance(ts, TestSuite):
            raise TypeError
    
        self.tests += ts.tests
        self.failures += ts.failures
        self.errors += ts.errors

        list.append(self, ts)


    def to_xml(self):

        elm = etree.Element('testsuites')
        
        for tc in self:
            elm.append(tc.to_xml())

        elm.attrib['name'] = self.name
        elm.attrib['tests'] = str(self.tests)
        elm.attrib['failures'] = str(self.failures)
        elm.attrib['errors'] = str(self.errors)
        elm.attrib['disabled'] = '0'
        elm.attrib['time'] = '0'

        return elm


class TestSuite(list):

    def __init__(self, name):
    
        list.__init__(self)
        self.name = name
        
        self.tests = 0
        self.failures = 0
        self.errors = 0


    def append(self, tc):
    
        if not isinstance(tc, TestCase):
            raise TypeError
    
        self.tests += 1
        if tc.status == TestCase.FAILURE:
            self.failures += 1
        elif tc.status == TestCase.ERROR:
            self.errors += 1

        list.append(self, tc)


    def to_xml(self):

        elm = etree.Element('testsuite')
        
        for tc in self:
            tc_xml = tc.to_xml()
            tc_xml.attrib['classname'] = self.name
            elm.append(tc_xml)

        elm.attrib['name'] = self.name
        elm.attrib['tests'] = str(self.tests)
        elm.attrib['failures'] = str(self.failures)
        elm.attrib['errors'] = str(self.errors)
        elm.attrib['disabled'] = '0'
        elm.attrib['time'] = '0'

        return elm


class TestCase:

    FAILURE = -1
    ERROR = 0
    SUCCESS = 1
    
    def __init__(self, name, status, message=None):
        
        self.name = name
        self.status = status
        self.message = message


    def to_xml(self):
    
        elm = etree.Element('testcase')
        elm.attrib['name'] = self.name
        elm.attrib['status'] = 'run'
        elm.attrib['time'] = '0'

        if self.status == TestCase.ERROR:
            elm.append(etree.Element('error', message=self.message))
        
        if self.status == TestCase.FAILURE:
            elm.append(etree.Element('failure', message=self.message))
            
        return elm

if __name__ == "__main__":
    # Parse command line parameters
    parser = argparse.ArgumentParser(description="Check integrity of the PyHMF API by comparing it to PyNN's common API.")
    parser.add_argument('--xml-path', '-x', action='store', dest='xml_path', help="path for the xml test sreport")
    parser.add_argument('--verbose', '-v', action='store_true', dest='verbose', help="show additional information")
    args = parser.parse_args()

    # Set up the XML report
    report = Report("Coverage")
    suite_top_level = TestSuite("TopLevelThingies")

    # Enter the main loop comparing PyHMF's API with the PyNN common API.
    # Looks broken but works fine. Taken from PyNN's tests.
    for item in dir(common):
        if item not in exclude_list:
            fm = getattr(common,item)
            if type(fm) == types.FunctionType:
                result = checkFunction(fm)
                writeln(item, result.to_cstring())
                if args.verbose and result.message:
                    print 8*" " + " | " + result.message
                if isinstance(result, Success):
                    suite_top_level.append(TestCase(item, TestCase.SUCCESS))
                else:
                    suite_top_level.append(TestCase(item, TestCase.ERROR, result.__class__.__name__))
            elif type(fm) == types.ClassType or type(fm) == types.TypeType:
                suite = TestSuite(item)
                
                result = checkClass(item)
                writeln(item, result.to_cstring())
                if args.verbose and result.message:
                    print 8*" " + " | " + result.message
                if isinstance(result, Success):
                    suite.append(TestCase(item, TestCase.SUCCESS))
                else:
                    suite.append(TestCase(item, TestCase.ERROR, result.__class__.__name__))
                for subitem in dir(fm):
                    if subitem not in exclude_list:
                        fm1 = getattr(fm,subitem)
                        if type(fm1) == types.MethodType:
                            result = checkMethod(fm1, item)
                            writeln(subitem, result.to_cstring(), 4)
                            if args.verbose and result.message:
                                print 8*" " + " | " + result.message
                            if isinstance(result, Success):
                                suite.append(TestCase(subitem, TestCase.SUCCESS))
                            else:
                                suite.append(TestCase(subitem, TestCase.ERROR, result.__class__.__name__))
                        elif type(fm1) == types.FunctionType:
                            result = checkStaticMethod(fm1, item)
                            writeln(subitem, result.to_cstring(), 4)
                            if args.verbose and result.message:
                                print 8*" " + " | " + result.message
                            if isinstance(result, Success):
                                suite.append(TestCase(subitem, TestCase.SUCCESS))
                            else:
                                suite.append(TestCase(subitem, TestCase.ERROR, result.__class__.__name__))
                        else:
                            pass
                            # TODO: Check class data!
                report.append(suite)
            else:
                result = checkData(item)
                writeln(item, result.to_cstring())
                if args.verbose and result.message:
                    print 8*" " + " | " + result.message
                if isinstance(result, Success):
                    suite_top_level.append(TestCase(item, TestCase.SUCCESS))
                else:
                    suite_top_level.append(TestCase(item, TestCase.ERROR, result.__class__.__name__))
    
    report.append(suite_top_level)
    
    # Write XML report if desired
    if args.xml_path:
        open(args.xml_path, 'w').write(etree.tostring(report.to_xml(), pretty_print=True))
