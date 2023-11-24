Utilities for wrapping functions taking/returning std::weak_ptr.

Pybind11 doesn't natively have support for functions that take a weak_ptr as
argument or return a weak_ptr, even when the shared_ptr is defined as holder
type and the type inherits from enable_shared_from_this.

For example, the following:

```
class Widget;
using WidgetSharedPtr = std::shared_ptr<Widget>;
using WidgetWeakPtr = std::weak_ptr<Widget>;

class Widget : public std::enable_shared_from_this<Widget> {
public:
    WidgetWeakPtr parent() const;
};

void wrap_widget(py::module& m) {
    py::class_<Widget, WidgetSharedPtr>()
        .def("parent", &Widget::parent);
}
```

would fail at runtime when calling `widget.parent()` with errors
similar to the following:

```
TypeError: Unregistered type : std::__1::weak_ptr<Widget>

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "cpp-py-smart-pointers-research/libs/x05/test.py", line 35, in testWidget
    widget.parent()
    ^^^^^^^^^^^^^^^
TypeError: Unable to convert function return value to a Python type! The signature was
  (arg0: x05.Widget) -> std::__1::weak_ptr<Action>
```

Wrapping these functions therefore requires more work. In `x03` and `x04`,
we've used a lambda to manually convert the weak_ptr into a shared_ptr:

```
    py::class_<Widget, WidgetSharedPtr>()
        .def("parent", [](Widget& self) -> WidgetSharedPtr {
            return self.parent().lock();
        });
```

In this experiment, we explore ways to make the process easier with utility
functions or macros.

Note that this experiment is not meant to fix the memory leak problems seen in
`x04`: we will still only hold shared_ptr on the Python side, which is prone
to memory leaks due to cyclic dependencies.

Also note that `py::implicitly_convertible` cannot help us here. It only works
for types which are actually wrapped (e.g., `Widget`), and here neither
`shared_ptr<Widget>` or `weak_ptr<Widget>` is a wrapped class. We will explore
in other experiement actually considering WidgetSharedPtr and/or WidgetWeakPtr to
be their own wrapped classes.
