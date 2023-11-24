#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "widget.h"

#define RET_SHARED(T, method) [](T& self) { return self.method().lock(); }

#define ARG_SHARED(T, method, U)                                                         \
    [](T& self, U& arg) { self.method(arg.weak_from_this()); }

// Note: for convenience, all the template free function below could be
// implement as member methods of a class that inherits py::class_, so we could
// directly do the usual:
//
//     cl.def(...)
//
// instead of:
//
//     def(cl, ...)
//

// Note: this needs to be a templated class with a static method, as opposed to
// a templated free function, because template partial specialization of free
// functions is not allowed.
//
template<typename T>
struct Transform {
    static T x(T t) {
        return t;
    }
};

template<typename T>
struct Transform<std::weak_ptr<T>> {
    static std::weak_ptr<T> x(T& t) {
        return t.weak_from_this();
    }
};

template<typename T>
struct ArgInfo {
    using CppType = T;
    using PyType = T;
};

template<typename T>
struct ArgInfo<std::weak_ptr<T>> {
    using CppType = std::weak_ptr<T>;
    using PyType = T&;
};

// invoke non-const method with transformed arguments
template<typename Self, typename Ret, typename... ArgInfos>
Ret invoke(
    Self& self,
    Ret (Self::*pm)(typename ArgInfos::CppType...),
    typename ArgInfos::PyType... args) {

    return (self.*pm)(Transform<typename ArgInfos::CppType>::x(args)...);
}

// invoke const method with transformed arguments
template<typename Self, typename Ret, typename... ArgInfos>
Ret invoke(
    Self& self,
    Ret (Self::*pm)(typename ArgInfos::CppType...) const,
    typename ArgInfos::PyType... args) {

    return (self.*pm)(Transform<typename ArgInfos::CppType>::x(args)...);
}

// wrap non-const method returning a weak_ptr
template<typename C, typename Self, typename T, typename... Args>
void def(C& cl, const char* name, std::weak_ptr<T> (Self::*pm)(Args...)) {
    cl.def(name, [pm](Self& self, typename ArgInfo<Args>::PyType... args) {
        return invoke<Self, std::weak_ptr<T>, ArgInfo<Args>...>(self, pm, args...).lock();
    });
}

// wrap const method returning a weak_ptr
template<typename C, typename Self, typename T, typename... Args>
void def(C& cl, const char* name, std::weak_ptr<T> (Self::*pm)(Args...) const) {
    cl.def(name, [pm](Self& self, typename ArgInfo<Args>::PyType... args) {
        return invoke<Self, std::weak_ptr<T>, ArgInfo<Args>...>(self, pm, args...).lock();
    });
}

// wrap non-const method not returning a weak_ptr
template<typename C, typename Self, typename T, typename... Args>
void def(C& cl, const char* name, T (Self::*pm)(Args...)) {
    cl.def(name, [pm](Self& self, typename ArgInfo<Args>::PyType... args) {
        return invoke<Self, T, ArgInfo<Args>...>(self, pm, args...);
    });
}

// wrap const method not returning a weak_ptr
template<typename C, typename Self, typename T, typename... Args>
void def(C& cl, const char* name, T (Self::*pm)(Args...) const) {
    cl.def(name, [pm](Self& self, typename ArgInfo<Args>::PyType... args) {
        return invoke<Self, T, ArgInfo<Args>...>(self, pm, args...);
    });
}

void wrap_widget(py::module& m) {
    py::class_<Widget, WidgetSharedPtr> cl(m, "Widget");

    cl //
        .def(py::init(&Widget::create))

        .def_property("name", &Widget::name, &Widget::setName)

        .def("getParentNaive", &Widget::parent)
        .def("setParentNaive", &Widget::parent)

        .def(
            "getParentManualLambda",
            [](Widget& self) { //
                return self.parent().lock();
            })
        .def(
            "setParentManualLambda",
            [](Widget& self, Widget& parent) { //
                self.setParent(parent.weak_from_this());
            })

        // The macro method works okay for simple method with only zero/one arguments,
        // but would requires very complex machinery for multiple arguments, some of
        // which being weak ptr to convert, and some being "normal" arguments. Would
        // probably have to look like this at usage site:
        //
        // .def("getAndSetWithArgs", RET_SHARED(
        //      Widget, getAndSetWithArgs, (WEAK, Widget), (WEAK, Widget), (int)))
        //
        .def("getParentMacro", RET_SHARED(Widget, parent))
        .def("setParentMacro", ARG_SHARED(Widget, setParent, Widget))

        .def_property_readonly("value", &Widget::value);

    def(cl, "getParentTemplate", &Widget::parent);
    def(cl, "setParentTemplate", &Widget::setParent);

    def(cl, "getWithArgs", &Widget::getWithArgs);
    def(cl, "getWithWeakArgs", &Widget::getWithWeakArgs);
    def(cl, "setWithWeakArgs", &Widget::setWithWeakArgs);
}

PYBIND11_MODULE(x05, m) {
    wrap_widget(m);
}
