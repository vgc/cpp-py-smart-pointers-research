#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "../common.h"
#include "action.h"

class Widget;
using WidgetSharedPtr = std::shared_ptr<Widget>;
using WidgetWeakPtr = std::weak_ptr<Widget>;

// Utility class for unit tests
class WidgetRefCounter {
public:
    size_t count() const {
        return widget_.use_count();
    }

private:
    friend Widget;
    WidgetWeakPtr widget_;
    WidgetRefCounter(Widget& widget);
};

// Layering: Action < Widget
//
// This means that:
// - Widget is allowed to store an ActionSharedPtr
// - Action is NOT allowed to store a WidgetWeakPtr (but can store an ActionWeakPtr)
//
// Problem: Action stores a type-erased std::function, which may internally
//          store a WidgetSharedPtr if we're not careful.
//
class API Widget : public std::enable_shared_from_this<Widget> {
    struct CreateKey {};

public:
    Widget(CreateKey) {
    }

    static WidgetSharedPtr create() {
        return std::make_shared<Widget>(CreateKey());
    }

    std::string_view name() const {
        return name_;
    }

    void setName(std::string_view name) {
        name_ = name;
    }

    ActionWeakPtr action() const {
        return action_;
    }

    // Note: should the guideline be that if we store the action (especially as
    // shared pointer), then the function should take an `ActionSharedPtr`
    // argument? What if it stores a weak pointer?
    //
    void setAction(Action& action) {
        action_ = action.shared_from_this();
    }

    void triggerAction() {
        if (ActionSharedPtr action = action_) {
            action->executeCallback();
        }
    }

    WidgetRefCounter refCounter() {
        return *this;
    }

private:
    std::string name_;
    ActionSharedPtr action_;
};

inline WidgetRefCounter::WidgetRefCounter(Widget& widget)
    : widget_(widget.weak_from_this()) {
}
