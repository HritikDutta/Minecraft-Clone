#pragma once

#include <cmath>
#include <ctime>
#include "core/types.h"
#include "core/compiler_utils.h"

template<typename T>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE constexpr T Abs(const T& a)
{
    return (a >= 0) ? a : -a;
}

// Max and Min Functions

template<typename T>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE constexpr T Min(const T& a, const T& b)
{
    return ((a < b) ? a : b);
}

template<typename T>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE constexpr T Max(const T& a, const T& b)
{
    return ((a > b) ? a : b);
}

template<typename T>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE T Clamp(const T& x, const T& min, const T& max)
{
    return Min(Max(x, min), max);
}

template<typename T>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE T Wrap(const T& x, const T& min, const T& max)
{
    T diff = x - min;
    T range = max - min;

    return (range + diff % range) % range + min;
}

template<>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Wrap(const f32& x, const f32& min, const f32& max)
{
    f32 diff = x - min;
    f32 range = max - min;

    return fmodf(range + fmodf(diff, range), range) + min;
}

template<>
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f64 Wrap(const f64& x, const f64& min, const f64& max)
{
    f64 diff = x - min;
    f64 range = max - min;

    return fmod(range + fmod(diff, range), range) + min;
}

namespace Math
{

// Convenience Functions

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 AlmostEquals(f32 a, f32 b, f32 epsilon = 0.001f)
{
    return Abs(a - b) <= epsilon;
}

// Math Functions

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Sign(f32 t)
{
    return __signbitvaluef(t);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Sin(f32 t)
{
    return sinf(t);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Cos(f32 t)
{
    return cosf(t);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Tan(f32 t)
{
    return tanf(t);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Exp(f32 x)
{
    return expf(x);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Log(f32 x)
{
    return logf(x);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Floor(f32 x)
{
    return floorf(x);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Ceil(f32 x)
{
    return ceilf(x);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Sqrt(f32 x)
{
    return sqrtf(x);
}

// Extra Functions

// Gives a random float in the range [0, 1)
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Random()
{
    return rand() / (f32) RAND_MAX;
}

} // namespace Math