#! /usr/bin/python2

import unittest
import random
import numpy
import pyhmf

class IterationTest(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)


    def test_population(self):

        size = random.randint(1, 1000)
        text = '%ds till the end' % size
        celltype = pyhmf.IF_cond_exp
        pop = pyhmf.Population(size, celltype, label=text)

        self.assertEqual(size, len(list(pop.__iter__())))
        self.assertEqual(size, len(list(pop.all())))
        self.assertEqual(size, len(pop))
        self.assertEqual(size, pop.size)
        self.assertEqual(text, pop.label)
        self.assertEqual(celltype, pop.celltype)


    def test_population_view(self):

        size = random.randint(1, 1000)
        text = '%ds till the end' % size
        celltype = pyhmf.IF_cond_exp
        selector = numpy.array([ random.choice([True, False]) for x in range(0, size) ])
        pop = pyhmf.Population(size, celltype, label=text)
        pv = pyhmf.PopulationView(pop, selector)
        size_selector = len(numpy.where(selector==True)[0])

        pynn_text = 'view of \"%ds till the end\" containing %d' % (size, size_selector)

        self.assertEqual(size_selector, len(list(pv.__iter__())))
        self.assertEqual(size_selector, len(list(pv.all())))
        self.assertEqual(size_selector, len(pv))
        self.assertEqual(size_selector, pv.size)
        self.assertEqual(pynn_text, pv.label)
        self.assertEqual(celltype, pv.celltype)

        pv2_label = "test custom label"
        pv2 = pyhmf.PopulationView(pop, selector,label = pv2_label)
        self.assertEqual(pv2_label, pv2.label)


    def test_assembly(self):

        size_a = random.randint(1, 1000)
        size_b = random.randint(1, 1000)

        pop_a = pyhmf.Population(size_a, pyhmf.IF_cond_exp)
        pop_b = pyhmf.Population(size_b, pyhmf.IF_cond_exp)
        pop_c = pyhmf.Population(1, pyhmf.IF_cond_exp)

        view = pop_b[0:random.randint(1, size_b)]

        text = '%ds till the end' % (size_a + size_b)

        construction_styles = [
            pyhmf.Assembly(pop_a, view, label = text),
            pyhmf.Assembly([pop_a, view], label = text),
            pyhmf.Assembly((pop_a, view), label = text)
        ]

        for asm in construction_styles:
            self.assertEqual(len(asm), len(list(asm.__iter__())))
            self.assertEqual(len(asm), len(list(asm.all())))
            self.assertEqual(size_a + len(view), len(asm))
            self.assertEqual(size_a + len(view), asm.size)
            self.assertEqual(text, asm.label)

        # some more tests ;)
        pyhmf.Assembly(pop_a),
        pyhmf.Assembly(pop_a, label = text),
        pyhmf.Assembly([pop_a]),
        pyhmf.Assembly([pop_a], label = text),
        pyhmf.Assembly(pop_c, label = text),

    def test_projection(self):
        size_a = random.randint(1, 1000)
        size_b = random.randint(1, 1000)

        pop_a = pyhmf.Population(size_a, pyhmf.IF_cond_exp)
        pop_b = pyhmf.Population(size_b, pyhmf.IF_cond_exp)

        pro = pyhmf.Projection(pop_a, pop_b, pyhmf.AllToAllConnector())

        # ECM: iteration over Connections?
        #self.assertEqual(len(pro), len(list(pro.__iter__())))
        #self.assertEqual(len(pro), len(list(pro.all())))
        self.assertEqual(size_a * size_b, len(pro))
        self.assertEqual(size_a * size_b, pro.size)


if __name__ == '__main__':
    unittest.main()
