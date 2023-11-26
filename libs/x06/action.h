#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include "../common.h"

using Callback = std::function<void(void)>;

class Action;
using ActionSharedPtr = std::shared_ptr<Action>;
using ActionWeakPtr = std::weak_ptr<Action>;

// Utility class for unit tests
class ActionRefCounter {
public:
    size_t count() const {
        return action_.use_count();
    }

private:
    friend Action;
    ActionWeakPtr action_;
    ActionRefCounter(Action& action);
};

class API Action : public std::enable_shared_from_this<Action> {
    struct CreateKey {};

public:
    Action(CreateKey) {
    }

    static ActionSharedPtr create() {
        return std::make_shared<Action>(CreateKey());
    }

    std::string_view name() const {
        return name_;
    }

    void setName(std::string_view name) {
        name_ = name;
    }

    void setCallback(Callback callback) {
        callback_ = std::move(callback);
    }

    void executeCallback() {
        callback_();
    }

    ActionRefCounter refCounter() {
        return *this;
    }

private:
    std::string name_;
    Callback callback_;
};

inline ActionRefCounter::ActionRefCounter(Action& action)
    : action_(action.weak_from_this()) {
}
