#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
using rvp = py::return_value_policy;

#include "action.h"
#include "widget.h"

void wrap_action(py::module& m) {
    py::class_<ActionRefCounter>(m, "ActionRefCounter")
        .def_property_readonly("count", &ActionRefCounter::count);

    py::class_<Action, ActionSharedPtr>(m, "Action")
        .def(py::init(&Action::create))
        .def_property("name", &Action::name, &Action::setName)
        .def("setCallback", &Action::setCallback)
        .def("executeCallback", &Action::executeCallback)
        .def("refCounter", &Action::refCounter);
}

void wrap_widget(py::module& m) {
    py::class_<WidgetRefCounter>(m, "WidgetRefCounter")
        .def_property_readonly("count", &WidgetRefCounter::count);

    py::class_<Widget, WidgetSharedPtr>(m, "Widget")
        .def(py::init(&Widget::create))
        .def_property("name", &Widget::name, &Widget::setName)
        .def_property("action", &Widget::action, &Widget::setAction)
        .def("triggerAction", &Widget::triggerAction)
        .def("refCounter", &Widget::refCounter);
}

PYBIND11_MODULE(x04, m) {
    wrap_action(m);
    wrap_widget(m);
}
