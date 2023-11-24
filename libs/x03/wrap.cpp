#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
using rvp = py::return_value_policy;

#include "tree.h"

// [1] Major issue:
//
// In `child` and `createChild`, we use reference_internal so that the child
// keeps alive `self` (that is, the parent node).
//
// This is at least better than the default (rvp::automatic) which would
// instead fallback to move/copy for lvalue references, which in our case would
// fail at runtime because we've disabled copy and move. We'd get errors like
// this issued by pybind11:
//
// RuntimeError: return_value_policy = copy, but type Node is non-copyable!
//
// However, even if it's better than the default doesn't mean it's great: we
// would instead prefer to keep alive the whole tree, not just the parent node!
// In some cases, we might be lucky since the parent keeps alive the parent,
// that keeps alive the parent, etc... and the root keeps alive the tree. But
// this assumes that Python has seen all of these as variables, and that they
// are still around.
//
// Perhaps more importantly, pybind11 only apply the policies when the object
// is first seen by pybind11. So it wouldn't work at all in case of reparenting
// the node or moving the nodes from one tree to another. See the Note after
// the table at
// https://pybind11.readthedocs.io/en/stable/advanced/functions.html#return-value-policies:
//
//   One important aspect of the above policies is that they only apply to
//   instances which pybind11 has not seen before, in which case the policy
//   clarifies essential questions about the return value’s lifetime and ownership.
//   When pybind11 knows the instance already (as identified by its type and address
//   in memory), it will return the existing Python object wrapper rather than
//   creating a new copy.
//
// [2] Also, by default, the methods returning weak_ptr wouldn't work, you'd
// see the error below. This is why we need to explicitly cast them. In this
// experiment (x03), we choose to convert them to SharedPtr. In x04, we attempt
// to keep them as WeakPtr.
//
//   ======================================================================
//   ERROR: testConstructor (__main__.TestTree.testConstructor)
//   ----------------------------------------------------------------------
//   TypeError: Unregistered type : std::__1::weak_ptr<Node>
//
//   The above exception was the direct cause of the following exception:
//
//   Traceback (most recent call last):
//     File "/Users/boris/cpp-py-smart-pointers-research/libs/x03/test.py", line 20, in testConstructor
//       self.assertEqual(tree.root.name, "root")
//                        ^^^^^^^^^
//   TypeError: Unable to convert function return value to a Python type! The signature was
//   	(arg0: x03.Tree) -> std::__1::weak_ptr<Node>
//

void wrap_node(py::module& m) {
    py::class_<Node, NodeSharedPtr>(m, "Node")

        // the tree should not keep alive the node, hence rvp::reference
        .def_property_readonly("tree", &Node::tree, rvp::reference)

        // the parent should not keep alive the child, hence rvp::reference
        .def_property_readonly(
            "parent",
            [](Node& self) -> NodeSharedPtr { return self.parent().lock(); },
            rvp::reference)

        // the rvp does not matter here: pybind11 will make a copy into a Python string
        .def_property("name", &Node::name, &Node::setName)

        // the rvp does not matter here: pybind11 will make a copy into a Python integer
        .def_property_readonly("numChildren", &Node::numChildren)

        // the returned child should keep alive its parent [1].
        .def(
            "child",
            [](Node& self, size_t i) -> NodeSharedPtr { return self.child(i).lock(); },
            rvp::reference_internal)

        // the created child should keep alive its parent [1].
        .def(
            "createChild",
            [](Node& self, std::string_view name) -> NodeSharedPtr {
                return self.createChild(name).lock();
            }, // [2]
            rvp::reference_internal)

        // the rvp does not matter here: no returned value
        .def("clearChildren", &Node::clearChildren);
}

void wrap_tree(py::module& m) {
    py::class_<Tree>(m, "Tree")

        // constructor
        .def(py::init<>())

        // the root should keep alive the tree (note: reference_internal is already the default
        // for def_property, but we write it anyway for clarifying intent)
        .def_property_readonly(
            "root",
            [](Tree& self) -> NodeSharedPtr { return self.root().lock(); }, // [2]
            rvp::reference_internal);
}

PYBIND11_MODULE(x03, m) {
    wrap_node(m);
    wrap_tree(m);
}
