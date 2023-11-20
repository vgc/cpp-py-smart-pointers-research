Tree structure using std::unique_ptr.

In theory, the simpler and most idiomatic way to implement a node-based
heap-allocated tree structure in C++ is to use `std::unique_ptr<Node>`, see for
example [Herb Sutter’s 2016
talk](https://youtu.be/JfmTagWcqoE?si=w6dQ11SeXKY_JQKZ&t=739).

This is basically an implementation of the above.

Unfortunately, this doesn’t work as soon as you want safe external observers
(either in C++ or Python).

Indeed, manipulating an observer of `Node*` or `Node&` is not memory safe (or
thread-safe) since it could be dangling at any moment as the `unique_ptr` might
be indirectly reset or destructed, for example:

```
void foo(Node& node) {
    if (Node* parent = node->parent()) {
        parent->clearChildren();
    }
    std::cout << node.name() << std::endl; // Segfault
}
```

Of course, in the above example, it’s “easy” to see that there is a bug and
hopefully it would be found by a human during code review or testing. But the
point is that this class of bugs cannot be detected with static analysis, and
often they do occur when there are several levels of indirection that makes the
bug less obvious: for example clicking on a button widget may invoke a callback
that indirectly destroys the widget (e.g., a “close button”).

This is not a hypothetical example but an actual crash that we've experienced.
Basically, a `Widget` did not survive calling its `Widget::onKeyPress()`
method, breaking the basic assumption that an object stays alive when we are
calling one of its method. Indeed a method is essentially a free function with
`Widget& self` as first argument, and the typical C++ guideline is therefore to
assume that `self` outlives the call of the function.

To summarize, because a `unique_ptr` can be `reset()` or destructed (e.g.,
removed from a `vector`), we generally cannot provide a strong guarantee that
the object survives, unlike when using stack-allocated objects:

```
Node node;
foo(node); // strong guarantee by the language that node outlives the call to foo
```

The only way to have such strong guarantee is if the `unique_ptr` is itself
stack-allocated, and if we make sure that no-one stores a reference/pointer to
the `unique_ptr` (otherwise, `reset()` could be called, or the `unique_ptr`
could be moved to another `unique_ptr` which is not stack-allocated):

```
auto node = std::make_unique<Node>;
foo(*node);
```

Note that all of this is similar to `vector<T>::iterator` being invalidated if
the vector is modified. In this case, the `Node& node` was invalidated because
the tree was modified. The problem is that in a complex UI system or Node-based
scene trees with callbacks, it's basically impossible to prove that the tree
will not change and therefore that the `Node& node` will still a valid
non-dangling reference.
