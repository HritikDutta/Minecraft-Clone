#pragma once

#include "core/types.h"
#include <limits>

namespace Math
{
    // Exponential
    constexpr f32 E    = 2.71828182845f;    // e

    // Angles
    constexpr f32 PI   = 3.14159265359f;    // pi
    constexpr f32 PI_2 = 1.57079632679f;    // pi/2
    
    constexpr f32 DegToRad = 0.01745329251f;
    constexpr f32 RadToDeg = 57.2957795131f;

#ifdef _MSC_VER
    static const f32 Infinity = __builtin_huge_valf();
#endif

#ifdef __GNUC__
    constexpr f32 Infinity = __builtin_inf();
#endif

} // namespace Math
