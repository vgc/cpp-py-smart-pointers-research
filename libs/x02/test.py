#!/usr/bin/python3

import unittest
from x02 import Node, Tree

def getRootOfNewTree():
    tree = Tree()
    return tree.root

def getNodeOfNewTree():
    tree = Tree()
    node = tree.root.createChild("node1")
    return node

class TestTree(unittest.TestCase):

    def testConstructor(self):
        tree = Tree()
        self.assertEqual(tree.root.name, "root")

    def testKeepAliveRootToTree(self):
        node = getRootOfNewTree()
        self.assertEqual(node.name, "root")

    # This test passes but it's unclear whether this behavior is guaranteed?
    def testKeepAliveNodeToTree(self):
        node = getNodeOfNewTree()
        self.assertEqual(node.name, "node1")

    # The following test is UB.
    #
    # When uncommenting it, not only the test may fail, possible output:
    #
    # 1/1 Test #2: x02_test.py ......................***Exception: SegFault  0.03 sec
    #
    # But in some cases, we've seen it fail less "violently", such as:
    #
    # "AssertionError: '' != 'node1')
    #
    # Or even making other unrelated tests fail.
    #
    #def testAccessingClearedChild(self):
    #    tree = Tree()
    #    root = tree.root
    #    node = root.createChild("node1")
    #    root.clearChildren()
    #    self.assertEqual(node.name, "node1")

if __name__ == '__main__':
    unittest.main()
