Tree structure using std::shared_ptr (minimal).

As we've seen in `x02`, using `unique_ptr` is not memory safe (or thread safe)
whenever you have observers: it is the responsibility of the code holding the
observer to ensure that no mutation occurs on the `Tree` while using a raw
pointer/reference to a `Node`. This is similar to `std::vector` or `std::list`
iterators which are "invalidated" when the container is modified, but the
problem is that for a complex system with callbacks, it is really hard to prove
and/or make sure that no invalidation does indeed occur.

The most obvious solution to this problem is to use `std::shared_ptr` instead,
which even have built-in support in pybind11. This x03 experiment is a minimal
(or "naive") implementation of a tree via `shared_ptr`, and we will see that
there are sill some problems.

Note that the usage guideline to ensure memory-safety is to never dereference
(either `*` or `->` operator) a `shared_ptr` without having a local copy on the
stack, as explained in [Herb Sutter’s 2014
talk](https://youtu.be/xnqTKD8uD64?si=o1laWjN78Qtbq03r&t=1380).

```
using NodeSharedPtr = std::shared_ptr<Node>;
using NodeWeakPtr = std::weak_ptr<Node>;

NodeSharedPtr sharedPtr; // global or data member
NodeWeakPtr weakPtr;     // global or data member

void doSomethingWithSharedPtr() {
    if (NodeSharedPtr pin = sharedPtr) {
        foo(*pin);
    }
}

void doSomethingWithWeakPtr() {
    if (NodeSharedPtr pin = weakPtr.lock()) {
        foo(*pin);
    }
}

void foo(Node& node) {
    if (NodeSharedPtr parent = node.parent().lock()) {
        parent->clearChildren();
    }
    std::cout << node.name() << std::endl; // No segfault possible if
                                           // caller followed pin convention
}
```

This is memory-safe, however there are a few things to be mindful of:

1. The Node class must now be designed and implemented to work even when
"isolated", that is, removed from the Tree. In particular, the `tree` back
pointer cannot anymore be a reference, since the node may outlive the tree. It
should either be a weak_ptr (if the tree itself is managed by a shared_ptr), or
a raw pointer set to nullptr when the node is removed from the tree. Same thing
for the the `parent` back pointer.

2. We have to be careful about avoiding memory leaks due to cyclic references
between `shared_ptr`. This cannot happen in this specific example, because when
a node is removed from a tree, all its `shared_ptr` data members are explicitly
removed. However, we show an example in x04 how it can happen.

3. The syntax to wrap functions that that return a `weak_ptr` (or take a
`weak_ptr` as argument) is a bit clunky as pybind11 wouldn't automatically
convert it to a `shared_ptr`. This might be automatable though with
`py::implicitly_convertible`

4. Finally, this doesn't solve the issue that by default, a Python variable to
a node wouldn't keep the Tree alive (we also have this issue with unique_ptr).
This requires to manually specify `return_value_policy`, but it doesn't work
well in case of reparenting: the thing to "keep alive" may change. We will
explore in other experiments how to solve this.
