#!/usr/bin/env python
import os, sys, random
sys.path.insert(0, 'lib')

from libester import *

a = create(IF_cond_exp, None, 10)

del a # we have to implement RAII :)

run(100)
