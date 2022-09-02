#pragma once

template <typename T>
void Swap(T& a, T& b)
{
    T t = a;
    a = b;
    b = t;
}