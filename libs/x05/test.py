#!/usr/bin/python3

import unittest
from x05 import Widget

class Tests(unittest.TestCase):

    def testName(self):
        widget = Widget()
        widget.name = "myWidget"
        self.assertEqual(widget.name, "myWidget")

    def testGetParentNaive(self):
        widget = Widget()
        self.assertRaises(TypeError, widget.parentNaive)
        # TypeError: Unregistered type : std::__1::weak_ptr<Widget>

if __name__ == '__main__':
    unittest.main()
