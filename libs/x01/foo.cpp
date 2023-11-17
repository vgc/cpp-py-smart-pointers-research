#include "foo.h"

Foo::Foo() {
}

int Foo::x() const {
    return x_;
}

void Foo::setX(int x) {
    x_ = x;
}
