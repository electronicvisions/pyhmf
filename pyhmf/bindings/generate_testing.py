#!/usr/bin/env python

from pywrap.wrapper import Wrapper

wrap = Wrapper()
mb = wrap.mb

mb.namespace('testing').include()

mb.add_declaration_code('#include "pyhmf/boost_python.h"')

# Finish wrapping
wrap.set_number_of_files(1)
wrap.finish()
