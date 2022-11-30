#pragma once

#include "core/types.h"
#include "containers/array_3d.h"
#include "containers/darray.h"
#include "math/vecs/vector3.h"
#include "aabb.h"
#include "voxel.h"

#include <SimplexNoise.h>

constexpr u32 CHUNK_SIZE = 32;

struct VoxelVertex;

using VoxelChunk = Array3D<BlockType>;

struct VoxelChunkArea
{
    DynamicArray<VoxelChunk> chunks;            // Data of the chunks in no particular order
    AABB* chunkBounds;                          // AABBs for each chunk (used for frustum culling)
    bool* isOnlyAir;                            // If the chunk is only air

    u32* opaqueFaceCounts;                      // Number of faces in each chunk's opaque mesh
    VoxelVertex** opaqueMeshData;               // Buffer containing opaque mesh data for a chunk
    
    u32* transparentFaceCounts;                 // Number of faces in each chunk's transparent mesh
    VoxelVertex** transparentMeshData;          // Buffer containing transparent mesh data for a chunk

    Array3D<u32> chunkIndices;                  // Index pointing to respective chunk data
    Array3D<u32> tempIndices;                   // Temporary Indices used for updating indices

    Vector3 areaPosition;                       // Position of center chunk in area
    f32 areaRadius;                             // Manhattan radius of area around player

    u32 newUpdatesLeft;                         // Number of new chunk mesh updates left
    u32 surrUpdatesLeft;                        // Number of surrounding chunk mesh updates left

    // Allocates the amount of data required for visible chunks
    void Create(f32 radius);
    void Free();

    void UpdateChunkMesh(u32 chunkX, u32 chunkY, u32 chunkZ);

    void InitializeChunkArea(const SimplexNoise& noise, const Vector3& position);
    void UpdateChunkArea(const SimplexNoise& noise, const Vector3& position);
};

void CorrectBlockIndex(Vector3Int& chunkIndex, Vector3Int& blockIndex);

void PlaceBlockAtPosition(VoxelChunkArea& area, const Vector3Int& chunkIndex, const Vector3Int& blockIndex, BlockType blockType);