#include "voxel_physics.h"

#include "intersections.h"
#include "chunk_renderer.h"
#include "voxel.h"

// Is the number in range [min, max)
inline bool IsInRange(f32 num, f32 min, f32 max)
{
    return num >= min && num < max;
}

bool RayIntersectionWithBlock(const VoxelChunkArea& area, const Vector3& rayOrigin, const Vector3& rayDirection, RayHitResult& hit, f32 maxDistance)
{
    Vector3 invRayDirection = Vector3(1.0f / rayDirection.x, 1.0f / rayDirection.y, 1.0f / rayDirection.z);

    {   // Check for intersecting chunk
        IntersectionResult blockClosestHit = {};

        const u32 halfDim = area.chunkIndices.dimension() / 2;
        for (u32 cz = halfDim - 2; cz < halfDim + 2; cz++)
        for (u32 cy = halfDim - 2; cy < halfDim + 2; cy++)
        for (u32 cx = halfDim - 2; cx < halfDim + 2; cx++)
        {
            u32 index = area.chunkIndices.at(cx, cy, cz);
            
            // Don't check for intersections in empty chunks
            if (area.isOnlyAir[index] || area.faceCounts[index] == 0)
                continue;

            // Offsets for chunk
            const f32 sx = ((f32) cx - halfDim);
            const f32 sy = ((f32) cy - halfDim);
            const f32 sz = ((f32) cz - halfDim);

            const Vector3 chunkPosition = area.areaPosition + Vector3(sx * CHUNK_SIZE, sy * CHUNK_SIZE, sz * CHUNK_SIZE);

            AABB chunkAABB;
            {   // Fill AABB
                chunkAABB.min = chunkPosition;
                chunkAABB.max = chunkAABB.min + Vector3(CHUNK_SIZE + 1);
            }

            IntersectionResult chunkHit;
            if (Intersection(chunkAABB, rayOrigin, invRayDirection, chunkHit, maxDistance))
            {
                const VoxelChunk& chunk = area.chunks[index];

                for (u32 z = 0; z < CHUNK_SIZE; z++)
                for (u32 y = 0; y < CHUNK_SIZE; y++)
                for (u32 x = 0; x < CHUNK_SIZE; x++)
                {
                    if (chunk.at(x, y, z) == BlockType::NONE)
                        continue;

                    AABB blockAABB;
                    {   // Fill AABB
                        blockAABB.min = chunkPosition + Vector3(x, y, z);
                        blockAABB.max = blockAABB.min + Vector3(1);
                    }

                    IntersectionResult blockHit;
                    if (Intersection(blockAABB, rayOrigin, invRayDirection, blockHit, maxDistance))
                    {
                        if (blockHit.tmax < blockClosestHit.tmax)
                        {
                            blockClosestHit = blockHit;
                            hit.chunkIndex = Vector3Int { (s32) cx, (s32) cy, (s32) cz };
                            hit.blockIndex = Vector3Int { (s32) x, (s32) y, (s32) z };
                            hit.t = blockHit.tmin;
                        }
                    }
                }
            }
        }
    }

    // Store point of hit
    hit.point = rayOrigin + hit.t * rayDirection;

    return hit.t < Math::Infinity;
}

AABB GetBlockAABB(const VoxelChunkArea& area, const Vector3Int& chunkIndex, const Vector3Int& blockIndex)
{
    const u32 halfDim = area.chunkIndices.dimension() / 2;
    
    // Offsets for chunk
    const f32 sx = ((f32) chunkIndex.x - halfDim);
    const f32 sy = ((f32) chunkIndex.y - halfDim);
    const f32 sz = ((f32) chunkIndex.z - halfDim);

    const Vector3 chunkPosition = area.areaPosition + Vector3(sx * CHUNK_SIZE, sy * CHUNK_SIZE, sz * CHUNK_SIZE);

    AABB blockAABB;
    {
        blockAABB.min = chunkPosition + Vector3(blockIndex.x, blockIndex.y, blockIndex.z);
        blockAABB.max = blockAABB.min + Vector3(1);
    }

    return blockAABB;
}

Vector3Int GetHitNormal(const VoxelChunkArea& area, const RayHitResult& hit)
{
    AABB blockAABB = GetBlockAABB(area, hit.chunkIndex, hit.blockIndex);

    // Find normal of the hit. This is done by finding the hit position of the ray on the cube.
    // Reference: https://blog.johnnovak.net/2016/10/22/the-nim-ray-tracer-project-part-4-calculating-box-normals/
    const Vector3 hitOnAABB   = hit.point - (blockAABB.min + Vector3(0.5f));
    const Vector3 normal = 1.001f * hitOnAABB / Vector3(0.5f);

    return Vector3Int { (s32) normal.x, (s32) normal.y, (s32) normal.z };
}