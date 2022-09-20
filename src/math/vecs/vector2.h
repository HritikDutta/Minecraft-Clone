#pragma once

#include "core/types.h"
#include "core/compiler_utils.h"
#include "../common.h"

union Vector2
{
    struct { f32 x, y; };
    struct { f32 u, v; };
    f32 data[2];

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2(f32 val = 0.0f)
    :   x(val), y(val)
    {
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2(f32 x, f32 y)
    :   x(x), y(y)
    {
    }

    // Vector Functions
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Length() const
    {
        return Math::Sqrt(x * x + y * y);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 SqrLength() const
    {
        return (x * x + y * y);
    }

    // Returns normalized vector without changing the original
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 Normalized() const
    {
        f32 len = Length();
        if (len != 0)
            return Vector2(x / len, y / len);
        
        return *this;
    }

    // Comparative Operators
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE bool operator==(const Vector2& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE bool operator!=(const Vector2& rhs) const
    {
        return (x != rhs.x) || (y != rhs.y);
    }

    // Unary Operator(s?)
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator-() const
    {
        return Vector2(-x, -y);
    }

    // Arithmetic Operators
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator+(const Vector2& rhs) const
    {
        return Vector2(x + rhs.x, y + rhs.y);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator-(const Vector2& rhs) const
    {
        return Vector2(x - rhs.x, y - rhs.y);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator*(const Vector2& rhs) const
    {
        return Vector2(x * rhs.x, y * rhs.y);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator/(const Vector2& rhs) const
    {
        return Vector2(x / rhs.x, y / rhs.y);
    }

    // op= Operators
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator+=(const Vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator-=(const Vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator*=(const Vector2& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator/=(const Vector2& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    // Arithmetic Operators with a Scalar
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator+(f32 scalar) const
    {
        return Vector2(x + scalar, y + scalar);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator-(f32 scalar) const
    {
        return Vector2(x - scalar, y - scalar);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator*(f32 scalar) const
    {
        return Vector2(x * scalar, y * scalar);
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator/(f32 scalar) const
    {
        return Vector2(x / scalar, y / scalar);
    }

    // op= Operators with a Scalar
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator+=(f32 scalar)
    {
        x += scalar;
        y += scalar;
        return *this;
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator-=(f32 scalar)
    {
        x -= scalar;
        y -= scalar;
        return *this;
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator*=(f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2& operator/=(f32 scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Indexing Operator
    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE const f32& operator[](int index) const
    {
        return data[Min(index, 1)];
    }

    GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32& operator[](int index)
    {
        return data[Min(index, 1)];
    }
};

// Arithmetic Operators with Scalar on the left
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator*(f32 scalar, const Vector2& vec)
{
    return vec * scalar;
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 operator/(f32 scalar, const Vector2& vec)
{
    return Vector2(scalar / vec.x, scalar / vec.y);
}

// Additional Functions
GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Dot(const Vector2& lhs, const Vector2& rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE f32 Cross(const Vector2& lhs, const Vector2& rhs)
{
    return (lhs.x * rhs.y - lhs.y * rhs.x);
}

GN_DISABLE_SECURITY_COOKIE_CHECK GN_FORCE_INLINE Vector2 Lerp(const Vector2& a, const Vector2& b, f32 t)
{
    f32 OneMinusT = 1.0f - t;
    return Vector2(OneMinusT * a.x + t * b.x, OneMinusT * a.y + t * b.y);
}