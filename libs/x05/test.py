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
        self.assertRaises(TypeError, widget.getParentNaive)
        # TypeError: Unregistered type : std::__1::weak_ptr<Widget>

    def testSetParentNaive(self):
        widget = Widget()
        parent = Widget()
        self.assertRaises(TypeError, widget.setParentNaive, parent)
        # TypeError: setParentNaive(): incompatible function arguments. The following argument types are supported:
        #     1. (self: x05.Widget) -> std::__1::weak_ptr<Widget>

    def testManualLambda(self):
        widget = Widget()
        parent = Widget()
        widget.setParentManualLambda(parent)
        self.assertEqual(widget.getParentManualLambda(), parent)

    def testMacro(self):
        widget = Widget()
        parent = Widget()
        widget.setParentMacro(parent)
        self.assertEqual(widget.getParentMacro(), parent)

    def testTemplate(self):
        widget = Widget()
        parent = Widget()
        widget.setParentTemplate(parent)
        self.assertEqual(widget.getParentTemplate(), parent)

        w2 = widget.getWithArgs(1, 2)
        self.assertEqual(w2, parent)
        self.assertEqual(widget.value, 3)

        w2 = widget.getWithWeakArgs(widget, parent, 1)
        self.assertEqual(w2, widget)
        self.assertEqual(widget.value, 1)

        w3 = widget.getWithWeakArgs(widget, parent, 2)
        self.assertEqual(w3, parent)
        self.assertEqual(widget.value, 2)

        widget.setWithWeakArgs(widget, parent, 1)
        self.assertEqual(widget.getParentTemplate(), widget)
        self.assertEqual(widget.value, 1)


if __name__ == '__main__':
    unittest.main()
