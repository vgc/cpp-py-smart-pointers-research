#pragma once

class Foo {
public:
    Foo(int x);
    int x() const;
    void setX(int x);

private:
    int x_;
};
