#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
using rvp = py::return_value_policy;

#include "action.h"
#include "widget.h"

// Equality comparison between a weak_ptr and:
// - another weak_ptr, or
// - a shared_ptr, or
// - a raw reference, or
// - a raw pointer.
//
// The last two assume that, if the pointer is not nullptr, then the object
// still exists, which is the standard assumption for all C++ function
// arguments.
//
// Note that in the general case, the equality comparison for shared_ptr /
// weak_ptr is made complicated due to the aliasing constructor of shared ptr
// (managing a pointer but refering to another one, i.e., nurse/patient
// keep-alive), which is why it is intentionally not defined in the standard
// library, see:
//
// See:
// - https://stackoverflow.com/questions/12301916/how-can-you-efficiently-check-whether-two-stdweak-ptr-pointers-are-pointing-to
// - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2004/n1590.html
//
// However, here, we assume that the objects inherit from
// `enable_shared_from_this` and are always constructed via `make_shared`.
// Therefore, there is no aliasing shared_ptr, and the address of the object is
// tied to the address of the control block (intrusing ref count). With those
// assumptions, the comparison operators below are all well-defined and
// thread-safe.
//
// Note: C++26 should introduce `owner_equal` (and `owner_hash`) which we could
// use to simplify some of the functions below. It would make sense to also
// implement these in our custom smart pointers. See:
//
// - https://isocpp.org/files/papers/P1901R2.html
// - https://github.com/cplusplus/papers/issues/649

template<typename T, typename U>
inline bool owner_equal(const std::weak_ptr<T>& t, const std::weak_ptr<U>& u) {
    return !t.owner_before(u) && !u.owner_before(t);
    // C++26: return t.owner_equal(u)
}

template<typename T, typename U>
inline bool owner_equal(const std::weak_ptr<T>& t, const std::shared_ptr<U>& u) {
    return !t.owner_before(u) && !u.owner_before(t);
    // C++26: return t.owner_equal(u)
}

template<typename T, typename U>
inline bool owner_equal(const std::shared_ptr<T>& t, const std::weak_ptr<U>& u) {
    return owner_equal(u, t);
}

template<typename T, typename U>
inline bool owner_equal(const std::weak_ptr<T>& t, const U& u) {
    return owner_equal(t, u.weak_from_this());

    // Note: u.weak_from_this() creates a temporary weak_ptr causing an
    // increase then decrease of the weak refcount. With a custom instrusive
    // shared pointer implementation, we could avoid this extra cost by
    // directly comparing the control block address. In fact, I think it could
    // be possible for the standard to add
    // `enable_shared_from_this<T>::owner_equal(...)`.
}

template<typename T, typename U>
inline bool owner_equal(const T& t, const std::weak_ptr<U>& u) {
    return owner_equal(u, t);
}

template<typename T, typename U>
inline bool owner_equal(const std::weak_ptr<T>& t, const U* u) {
    if (u) {
        return owner_equal(t, *u);
    }
    else {
        std::shared_ptr<U> nullSharedPtr;
        return owner_equal(t, nullSharedPtr);
    }
}

template<typename T, typename U>
inline bool owner_equal(const T* t, const std::weak_ptr<U>& u) {
    return owner_equal(u, t);
}

template<typename T, typename PyClass>
void wrap_weak_and_shared_from_this(PyClass& c) {
    c.def("toShared", py::overload_cast<>(&T::shared_from_this));
    c.def("toWeak", py::overload_cast<>(&T::weak_from_this));

    // Note: we need overload_cast to disambiguate between the const and
    // non-const version of shared_from_this (same for weak_from_this), see:
    //
    // https://en.cppreference.com/w/cpp/memory/enable_shared_from_this/shared_from_this
    //
    // ```
    // std::shared_ptr<T> shared_from_this();
    // std::shared_ptr<T const> shared_from_this() const;
    // ```
}

template<typename T>
void wrap_weak_ptr(py::module& m, const char* className) {

    using TWeakPtr = std::weak_ptr<T>;
    using TSharedPtr = std::shared_ptr<T>;

    auto getattribute =
        py::module::import("builtins").attr("object").attr("__getattribute__");

    auto setattr = py::module::import("builtins").attr("object").attr("__setattr__");

    std::string weakPtrName = className;
    weakPtrName += "WeakPtr";

    py::class_<TWeakPtr>(m, weakPtrName.c_str())
        .def("refCount", &TWeakPtr::use_count)
        .def(
            "__getattribute__",
            [getattribute](TWeakPtr& weakPtr, py::str name) {
                if (TSharedPtr sharedPtr = weakPtr.lock()) {
                    return getattribute(*sharedPtr, name);
                    // XXX: when `name` is a function/callable, I suppose getattribute only
                    //      returns the function itself, without calling it yet. How to keep alive
                    //      the sharedPtr until the end of the function call? Is it even possible?
                    // Idea: detect if the returned value of getattribute is a callable, and if
                    // so, return a new callable that stores a copy of the sharedPtr.
                }
                else {
                    throw std::logic_error(
                        "Cannot get attribute of object: the object is "
                        "not alive anymore.");
                }
            })
        .def(
            "__setattr__", // Note: there is no "__setattribute__". Shouldn't we also use "__getattr__"?
            [setattr](TWeakPtr& weakPtr, py::str name, py::object value) {
                if (TSharedPtr sharedPtr = weakPtr.lock()) {
                    return setattr(*sharedPtr, name, value);
                }
                else {
                    throw std::logic_error(
                        "Cannot set attribute of object: the object is "
                        "not alive anymore.");
                }
            })
        .def(
            "__eq__",
            [](const TWeakPtr& a, const TWeakPtr& b) { return owner_equal(a, b); },
            py::is_operator())
        .def(
            "__eq__",
            [](const TWeakPtr& a, const T& b) { return owner_equal(a, b); },
            py::is_operator());

    // Note: for some reason, it doesn't seem necessary to implement the mirror
    // equality operator:
    //
    //   [](const T& a, const TWeakPtr& b) { return owner_equal(a, b); }
    //
    // It's probably pybind11 that already tests if the reverse operator exists
}

void wrap_action(py::module& m) {

    py::class_<ActionRefCounter>(m, "ActionRefCounter")
        .def_property_readonly("count", &ActionRefCounter::count);

    py::class_<Action, ActionSharedPtr> c(m, "Action");
    c.def(py::init(&Action::create))
        .def_property("name", &Action::name, &Action::setName)
        .def("setCallback", &Action::setCallback)
        .def("executeCallback", &Action::executeCallback)
        .def("refCounter", &Action::refCounter);

    wrap_weak_ptr<Action>(m, "Action");
    wrap_weak_and_shared_from_this<Action>(c);
}

void wrap_widget(py::module& m) {

    py::class_<WidgetRefCounter>(m, "WidgetRefCounter")
        .def_property_readonly("count", &WidgetRefCounter::count);

    py::class_<Widget, WidgetSharedPtr> c(m, "Widget");
    c.def(py::init(&Widget::create))
        .def_property("name", &Widget::name, &Widget::setName)
        .def_property("action", &Widget::action, &Widget::setAction)
        .def("triggerAction", &Widget::triggerAction)
        .def("refCounter", &Widget::refCounter);

    wrap_weak_ptr<Widget>(m, "Widget");
    wrap_weak_and_shared_from_this<Widget>(c);
}

PYBIND11_MODULE(x06, m) {
    wrap_action(m);
    wrap_widget(m);
}
