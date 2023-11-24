#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "../common.h"

class Widget;
using WidgetSharedPtr = std::shared_ptr<Widget>;
using WidgetWeakPtr = std::weak_ptr<Widget>;

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

    WidgetWeakPtr parent() const {
        return parent_;
    }

    // This is for example only. With classes that inherit from
    // enable_shared_from_this, we think it is better practice to simply pass
    // the argument as `Widget& parent` or `Widget* parent` (if can be
    // nullable).
    //
    void setParent(WidgetWeakPtr parent) {
        parent_ = parent;
    }

private:
    std::string name_;
    WidgetWeakPtr parent_;
};
