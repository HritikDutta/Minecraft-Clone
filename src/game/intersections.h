#pragma once

#include "math/math.h"
#include "aabb.h"
#include "voxel.h"

struct IntersectionResult
{
    f32 tmin = -Math::Infinity;
    f32 tmax =  Math::Infinity;
};

bool Intersection(const AABB& aabb, const Vector3& rayOrigin, const Vector3& invRayDir, IntersectionResult& hit, f32 maxDistance = Math::Infinity)
{
    {   // Check with X axis
        const f32 t1 = (aabb.min.x - rayOrigin.x) * invRayDir.x;
        const f32 t2 = (aabb.max.x - rayOrigin.x) * invRayDir.x;

        hit.tmin = Max(Min(t1, t2), 0.0f);
        hit.tmax = Max(t1, t2);
    }

    {   // Check with Y axis
        const f32 t1 = (aabb.min.y - rayOrigin.y) * invRayDir.y;
        const f32 t2 = (aabb.max.y - rayOrigin.y) * invRayDir.y;

        const f32 currtmin = Max(Min(t1, t2), 0.0f);
        const f32 currtmax = Max(t1, t2);

        hit.tmin = Max(hit.tmin, currtmin);
        hit.tmax = Min(hit.tmax, currtmax);
    }

    {   // Check with Z axis
        const f32 t1 = (aabb.min.z - rayOrigin.z) * invRayDir.z;
        const f32 t2 = (aabb.max.z - rayOrigin.z) * invRayDir.z;

        const f32 currtmin = Max(Min(t1, t2), 0.0f);
        const f32 currtmax = Max(t1, t2);

        hit.tmin = Max(hit.tmin, currtmin);
        hit.tmax = Min(hit.tmax, currtmax);
    }

    return hit.tmax >= Max(hit.tmin, 0.0f) && hit.tmin <= maxDistance;
}