#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "foo.h"

void wrap_foo(py::module& m) {
    py::class_<Foo>(m, "Foo") //
        .def(py::init<>())
        .def_property("x", &Foo::x, &Foo::setX);
}

PYBIND11_MODULE(x01, m) {
    wrap_foo(m);
}
