#!/usr/bin/env python
# -*- coding: utf8 -*-

import unittest, xmlrunner
from pyhmf import InvalidParameterValueError, NonExistentParameterError,\
    InvalidDimensionsError, ConnectionError, InvalidModelError, \
    NoModelAvailableError, NothingToWriteError, InvalidWeightError, \
    NotLocalError, RecordingError    
import pyhmf_testing

class Exceptions(unittest.TestCase):

    def test_InvalidParameterValueError(self):
    
        with self.assertRaises(InvalidParameterValueError):
            pyhmf_testing.test_InvalidParameterValueError()


    def test_(self):
    
        with self.assertRaises(NonExistentParameterError):
            pyhmf_testing.test_NonExistentParameterError()


    def test_InvalidDimensionsError(self):
    
        with self.assertRaises(InvalidDimensionsError):
            pyhmf_testing.test_InvalidDimensionsError()


    def test_ConnectionError(self):
    
        with self.assertRaises(ConnectionError):
            pyhmf_testing.test_ConnectionError()


    def test_InvalidModelError(self):
    
        with self.assertRaises(InvalidModelError):
            pyhmf_testing.test_InvalidModelError()


    def test_NoModelAvailableError(self):
    
        with self.assertRaises(NoModelAvailableError):
            pyhmf_testing.test_NoModelAvailableError()


    def test_NothingToWriteError(self):
    
        with self.assertRaises(NothingToWriteError):
            pyhmf_testing.test_NothingToWriteError()


    def test_InvalidWeightError(self):
    
        with self.assertRaises(InvalidWeightError):
            pyhmf_testing.test_InvalidWeightError()


    def test_NotLocalError(self):
    
        with self.assertRaises(NotLocalError):
            pyhmf_testing.test_NotLocalError()


    def test_RecordingError(self):
    
        with self.assertRaises(RecordingError):
            pyhmf_testing.test_RecordingError()


if __name__ == '__main__':
    unittest.main()
