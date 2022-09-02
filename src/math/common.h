#pragma once

#include <cmath>
#include <ctime>
#include "core/types.h"

template<typename T>
inline constexpr T Abs(const T& a)
{
    return (a >= 0) ? a : -a;
}

// Max and Min Functions

template<typename T>
inline constexpr T Min(const T& a, const T& b)
{
    return ((a < b) ? a : b);
}

template<typename T>
inline constexpr T Max(const T& a, const T& b)
{
    return ((a > b) ? a : b);
}

template<typename T>
inline T Clamp(const T& x, const T& min, const T& max)
{
    return Min(Max(x, min), max);
}

template<typename T>
inline T Wrap(const T& x, const T& min, const T& max)
{
    T diff = x - min;
    T range = max - min;

    return (range + diff % range) % range + min;
}

template<>
inline f32 Wrap(const f32& x, const f32& min, const f32& max)
{
    f32 diff = x - min;
    f32 range = max - min;

    return fmodf(range + fmodf(diff, range), range) + min;
}

template<>
inline f64 Wrap(const f64& x, const f64& min, const f64& max)
{
    f64 diff = x - min;
    f64 range = max - min;

    return fmod(range + fmod(diff, range), range) + min;
}

namespace Math
{

// Convenience Functions

inline f32 AlmostEquals(f32 a, f32 b, f32 epsilon = 0.001f)
{
    return Abs(a - b) <= epsilon;
}

// Math Functions

inline f32 Sign(f32 t)
{
    return __signbitvaluef(t);
}

inline f32 Sin(f32 t)
{
    return sinf(t);
}

inline f32 Cos(f32 t)
{
    return cosf(t);
}

inline f32 Tan(f32 t)
{
    return tanf(t);
}

inline f32 Exp(f32 x)
{
    return expf(x);
}

inline f32 Log(f32 x)
{
    return logf(x);
}

inline f32 Floor(f32 x)
{
    return floorf(x);
}

inline f32 Ceil(f32 x)
{
    return ceilf(x);
}

inline f32 Sqrt(f32 x)
{
    return sqrtf(x);
}

// Extra Functions

// Gives a random float in the range [0, 1)
inline f32 Random()
{
    return rand() / (f32) RAND_MAX;
}

// TODO: Add more functions as needed...

} // namespace Math