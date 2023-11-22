#!/usr/bin/python3

import unittest
from x03 import Node, Tree

def getRootOfNewTree():
    tree = Tree()
    return tree.root

def getNodeOfNewTree():
    tree = Tree()
    node = tree.root.createChild("node1")
    return node

class TestTree(unittest.TestCase):

    def testConstructor(self):
        self.assertTrue(1+1, 2)
        tree = Tree()
        self.assertEqual(tree.root.name, "root")

    def testKeepAliveRootToTree(self):
        node = getRootOfNewTree()
        self.assertEqual(node.name, "root")

    # This test passes but it's unclear whether this behavior is guaranteed?
    def testKeepAliveNodeToTree(self):
        node = getNodeOfNewTree()
        self.assertEqual(node.name, "node1")

    # The following test is not UB anymore! (compared to x02)
    # In fact, it runs with no warnings/errors, while it would
    # be nice to have some sort of `node removed from tree` error.
    #
    def testAccessingClearedChild(self):
        tree = Tree()
        root = tree.root
        node = root.createChild("node1")
        root.clearChildren()
        self.assertEqual(node.name, "node1")

if __name__ == '__main__':
    unittest.main()
