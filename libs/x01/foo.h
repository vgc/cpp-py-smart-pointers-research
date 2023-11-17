#pragma once

#include "../common.h"

class API Foo {
public:
    Foo();
    int x() const;
    void setX(int x);

private:
    int x_ = 0;
};
