#!/usr/bin/python3

import unittest
from x01 import Foo

class TestFoo(unittest.TestCase):

    def testConstructor(self):
        foo = Foo()
        self.assertEqual(foo.x, 0)

    def testX(self):
        foo = Foo()
        foo.x = 12
        self.assertEqual(foo.x, 12)

if __name__ == '__main__':
    unittest.main()
