#pragma once

#include "compiler_utils.h"

template <typename T>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE void Swap(T& a, T& b)
{
    T t = a;
    a = b;
    b = t;
}