#pragma once

#include "core/types.h"
#include "containers/array_3d.h"
#include "containers/darray.h"
#include "engine/camera.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "aabb.h"
#include "voxel.h"

#include <SimplexNoise.h>

constexpr u32 CHUNK_SIZE = 32;

using VoxelChunk = Array3D<BlockType>;

struct DebugRendererStats
{
    u32 trianglesRendered;
    u32 batches;
};

struct DebugRendererSettings
{
    bool showWireframe = false;
    bool showBatches = false;
    bool showLighting = false;
};

struct VoxelVertex;
struct TransparentVoxel;

struct VoxelChunkArea
{
    DynamicArray<VoxelChunk> chunks;        // Data of the chunks in no particular order
    DynamicArray<AABB> chunkBounds;         // AABB for each chunk (used for frustum culling and sorting)
    DynamicArray<bool> isOnlyAir;           // If the chunk is only air
    DynamicArray<u32> faceCounts;           // Number of faces in each chunk mesh
    DynamicArray<VoxelVertex*> meshData;    // Buffer containing mesh data for a chunk

    Array3D<u32> chunkIndices;              // Index pointing to respective chunk data
    Array3D<u32> tempIndices;               // Temporary Indices used for updating indices

    Vector3 areaPosition;                   // Position of center chunk in area
    f32 areaRadius;                         // Manhattan radius of area around player

    u32 newUpdatesLeft;                     // Number of new chunk mesh updates left
    u32 surrUpdatesLeft;                    // Number of surrounding chunk mesh updates left

    // Allocates the amount of data required for visible chunks
    void Create(f32 radius);
    void Free();

    void UpdateChunkMesh(u32 chunkX, u32 chunkY, u32 chunkZ);

    void InitializeChunkArea(const SimplexNoise& noise, const Vector3& position);
    void UpdateChunkArea(const SimplexNoise& noise, const Vector3& position);
};

namespace ChunkRenderer
{

void Init();
void Shutdown();

void Begin(Camera& camera, const Texture& texture);
void End();

void RenderChunkArea(VoxelChunkArea& area, Shader& shader, DebugRendererStats& stats, const DebugRendererSettings& settings);

} // namespace ChunkRenderer