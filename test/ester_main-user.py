#!/usr/bin/env python
import os, sys, random
sys.path.insert(0, 'lib')

from libeuter import *

setup()
NPOP = 100000
NPRO = 100000
popus = [Population(random.randint(10,100), IF_brainscales_hardware) for x in range(0,NPOP)]
projs = [Projection(popus[random.randint(0,NPOP-1)], popus[random.randint(0,NPOP-1)], random.random()) for x in range(0,NPRO)]
run(1000)

setup()
reset()
reset()
end()
reset()

run(1000)
run(1000)

end()

setup()
setup()

# if we don't do the assignment => destructor gets called... :(
#a = create(IF_cond_exp, n=10)
#b = create(IF_cond_exp, n=20)
#c = connect(a, b, p=1.0)

run(10)

a = Population(10, IF_brainscales_hardware)
a.set("tau_m", 5.0)
print(a)
a.set({"tau_m": 5.5})
print(a)
