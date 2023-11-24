#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "widget.h"

void wrap_widget(py::module& m) {
    py::class_<Widget, WidgetSharedPtr>(m, "Widget")
        .def(py::init(&Widget::create))
        .def_property("name", &Widget::name, &Widget::setName)
        .def("parentNaive", &Widget::parent);
}

PYBIND11_MODULE(x05, m) {
    wrap_widget(m);
}
