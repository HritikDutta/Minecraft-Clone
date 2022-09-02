#pragma once

#include "core/types.h"
#include "../common.h"

union Vector2
{
    struct { f32 x, y; };
    struct { f32 u, v; };
    f32 data[2];

    Vector2(f32 val = 0.0f)
    :   x(val), y(val)
    {
    }

    Vector2(f32 x, f32 y)
    :   x(x), y(y)
    {
    }

    // Vector Functions
    inline f32 Length() const
    {
        return Math::Sqrt(x * x + y * y);
    }

    inline f32 SqrLength() const
    {
        return (x * x + y * y);
    }

    // Returns normalized vector without changing the original
    inline Vector2 Normalized() const
    {
        f32 len = Length();
        if (len != 0)
            return Vector2(x / len, y / len);
        
        return *this;
    }

    // Comparative Operators
    inline bool operator==(const Vector2& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }

    inline bool operator!=(const Vector2& rhs) const
    {
        return (x != rhs.x) || (y != rhs.y);
    }

    // Unary Operator(s?)
    inline Vector2 operator-() const
    {
        return Vector2(-x, -y);
    }

    // Arithmetic Operators
    inline Vector2 operator+(const Vector2& rhs) const
    {
        return Vector2(x + rhs.x, y + rhs.y);
    }

    inline Vector2 operator-(const Vector2& rhs) const
    {
        return Vector2(x - rhs.x, y - rhs.y);
    }

    inline Vector2 operator*(const Vector2& rhs) const
    {
        return Vector2(x * rhs.x, y * rhs.y);
    }

    inline Vector2 operator/(const Vector2& rhs) const
    {
        return Vector2(x / rhs.x, y / rhs.y);
    }

    // op= Operators
    inline Vector2& operator+=(const Vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    inline Vector2& operator-=(const Vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    inline Vector2& operator*=(const Vector2& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    inline Vector2& operator/=(const Vector2& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    // Arithmetic Operators with a Scalar
    inline Vector2 operator+(f32 scalar) const
    {
        return Vector2(x + scalar, y + scalar);
    }

    inline Vector2 operator-(f32 scalar) const
    {
        return Vector2(x - scalar, y - scalar);
    }

    inline Vector2 operator*(f32 scalar) const
    {
        return Vector2(x * scalar, y * scalar);
    }

    inline Vector2 operator/(f32 scalar) const
    {
        return Vector2(x / scalar, y / scalar);
    }

    // op= Operators with a Scalar
    inline Vector2& operator+=(f32 scalar)
    {
        x += scalar;
        y += scalar;
        return *this;
    }

    inline Vector2& operator-=(f32 scalar)
    {
        x -= scalar;
        y -= scalar;
        return *this;
    }

    inline Vector2& operator*=(f32 scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    inline Vector2& operator/=(f32 scalar)
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Indexing Operator
    inline const f32& operator[](int index) const
    {
        return data[Min(index, 1)];
    }

    inline f32& operator[](int index)
    {
        return data[Min(index, 1)];
    }
};

// Arithmetic Operators with Scalar on the left
inline Vector2 operator*(f32 scalar, const Vector2& vec)
{
    return vec * scalar;
}

inline Vector2 operator/(f32 scalar, const Vector2& vec)
{
    return Vector2(scalar / vec.x, scalar / vec.y);
}

// Additional Functions
inline f32 Dot(const Vector2& lhs, const Vector2& rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y);
}

inline f32 Cross(const Vector2& lhs, const Vector2& rhs)
{
    return (lhs.x * rhs.y - lhs.y * rhs.x);
}

inline Vector2 Lerp(const Vector2& a, const Vector2& b, f32 t)
{
    f32 OneMinusT = 1.0f - t;
    return Vector2(OneMinusT * a.x + t * b.x, OneMinusT * a.y + t * b.y);
}