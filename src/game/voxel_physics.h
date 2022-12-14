#pragma once

#include "chunk_renderer.h"
#include "voxel.h"

struct RayHitResult
{
    Vector3Int chunkIndex;
    Vector3Int blockIndex;
    f32 t = Math::Infinity;
    Vector3 point;
};

bool RayIntersectionWithBlock(const VoxelChunkArea& area, const Vector3& rayOrigin, const Vector3& rayDirection,
                              RayHitResult& hit, f32 maxDistance = Math::Infinity);

Vector3Int GetHitNormal(const VoxelChunkArea& area, const RayHitResult& hit);