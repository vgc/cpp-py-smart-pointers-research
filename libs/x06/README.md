Fix leaks by exposing std::weak_ptr as separate Python class

This presents one method of allowing Python code to fix the memory leaks shown
in `x04` by explicitly exposing std::weak_ptr as a separate Python class.

This way, Python users can specify whether a Python variable takes ownership of
the underlying object (using the default std::shared_ptr holder), or if they
are just an observer).

```
>>> action = Action()
>>> type(action)
<class 'x06.Action'>           # internally storing a shared_ptr
>>> action_w = action.toWeak()
>>> type(action_w)
<class 'x06.ActionWeakPtr'>    # internally storing a weak_ptr
>>> action.name = "hello"
>>> print(action_w.name)
hello
```

The class `ActionWeakPtr` works by reimplementing `__getattribute__` and
`__setattr__`, taking a lock, then calling `__getattribute__` / `__setattr__`
on the underlying `Action` PyObject managed by pybind11.

Custom equality operators between Action and ActionWeakPtr are implemented such
that they return true if and only if their underlying action instance is the
same.

Changing the following code from `x04`

```
action.setCallback(lambda x = widget : changeName(x))
```

to 

```
action.setCallback(lambda x = widget.toWeak() : changeName(x))
```

fixes the memory leak :)

Note that memory leaks are not auto-magically prevented: by using the 
same code as `x04`, you still get the memory leak. You need to explicitly
call `toWeak()` in this case.

However, the point is that this approach:
- At least makes it *possible* to change the Python code minimally to fix the leak.
- Should eliminate most memory leaks by default if using a coding style that mostly
  use weak pointers rather than shared pointers.
  
Also, note that in this experiment, `shared_ptr<T>` is the holder type
("idiomatic pybind11 style"), and `weak_ptr<T>` is the separately wrapped
class. By doing the opposite, it should reduce even more the risk of cyclic
dependencies of shared pointers.

Also, in some cases, the `shared_ptr<T>` version might even not be exposed at
all to Python users for more safety. This would be an option whenever new
instances should only be created and owned by C++ code, for example the `Node`
instances of a `Tree` class should only be created by the tree itself, and only
be "observed" by client code. In this case, C++ clients would use `Node&`,
`Node*`, `NodeHandle` and `NodeHandleLock` (the last two being equivalent to
`weak_ptr` and `shared_ptr`, but with names communicating the intent that the
lock is only supposed to be held temporarily for memory safety, and not stored
as data member), while in Python, there would just be `Node`, that would under
the hood store a `NodeHandle` and create the temporary lock in
`__getattribute__`, `__setattr__`.
