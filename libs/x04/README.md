Memory leaks with std::shared_ptr: Action/Widget cyclic dependency

As we've seen in `x03`, using `shared_ptr` can make access to nodes of a tree
memory-safe.

However, using `shared_ptr` isn't an easy answer for all problems, and in
particular, one must be careful about avoiding cyclic dependencies to avoid
memory leaks. In [Herb Sutter’s 2016 talk](https://youtu.be/JfmTagWcqoE), a
solution that is mentionned is to not break "layering", that is, have a clear
non-cyclic hierarchy of classes, and only allowing "higher-level classes" to
store pointers of "lower-level classes".

Unfortunately, this is hard to apply when there is any sort of dependency
injection, for example a callback mechanism. In C++, this can be solve by
insuring that all callbacks/lambdas stored in lower-level classes reference
higher-level objects via weak pointers.

Here, we show an example of such cyclic dependency, with a class Widget and
a class Action:
- The C++ Widget class owns an Action by storing an `std::shared_ptr<Action>`
- The C++ Action class owns a callback by storing an `std::function<void(void)>`
- We wrap these classes in Python using the most idiomatic pybind11 code
- In Python, we create a Widget instance and an Action instance
  whose callback modifies to the widget's name
- This creates a memory leak due to cyclic dependency:
  - The widget instance stores a `shared_ptr<Action>` in the Widget::action_ data member
  - the action instance stores a PyObject that indirectly stores a `shared_ptr<Widget>`
  
