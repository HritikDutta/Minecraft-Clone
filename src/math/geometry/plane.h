#pragma once

#include "core/types.h"
#include "../vecs/vector3.h"

union Plane
{
    struct { f32 a, b, c, d; };
    Vector4 vector;

    Plane() {  }

    Plane(const Vector4& v)
    :   vector(v)
    {
        Vector3 normal = Vector3(vector.x, vector.y, vector.z);
        vector /= normal.Length();
    }

    f32 EvaluatePoint(Vector3 point) const
    {
        Vector4 p = Vector4(point.x, point.y, point.z, 1.0f);
        return Dot(vector, p);
    }
};