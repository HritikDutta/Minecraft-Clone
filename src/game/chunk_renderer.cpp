#include "chunk_renderer.h"

#include "core/types.h"
#include "containers/darray.h"
#include "containers/hashtable.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "engine/camera.h"
#include "platform/platform.h"
#include "voxel.h"
#include "voxel_ao.h"
#include "voxel_renderdata.h"

#include <glad/glad.h>

struct VoxelVertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    float occlusion;
};

constexpr u32 maxVoxelFaceCount = 1 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr u32 maxFacesInBatch = 4 * maxVoxelFaceCount;
constexpr u64 maxChunkBatchSize = maxFacesInBatch * sizeof(VoxelVertex);
constexpr u32 chunkUpdatesPerFrame = 3;

struct ChunkUpdateData
{
    u32 index;
    u32 x, y, z;

    bool operator==(const ChunkUpdateData& other)
    {
        return x == other.x &&
               y == other.y &&
               z == other.z;
    }
};

struct
{
    // Using common vbo, vao, and ibo since we're batching meshes before drawing them
    u32 vbo, vao, iboMesh, iboWireframe;

    Camera* camera = nullptr;

    DynamicArray<ChunkUpdateData> surroundingChunkUpdateList;
    DynamicArray<ChunkUpdateData> newChunkUpdateList;

    s32 aoXOffsets[((6 << 4) | 8)][3];
    s32 aoYOffsets[((6 << 4) | 8)][3];
    s32 aoZOffsets[((6 << 4) | 8)][3];
} crData;

/*

Performance notes just for me:
- 1022952 tris - no culling
-  223560 tris - with intra chunk culling
-  199204 tris - with inter chunk culling
-   69292 tris - with frustum culling (avg)
-   74906 tris - with culling further chunks (avg)
-   45236 tris - with culling further chunks and frustum culling (avg)

This was with random generation of blocks in a chunk.
These were the properties for world generation.

constexpr u32 CHUNK_SIZE = 16;
constexpr u32 CHUNK_SIZE = 16;
constexpr u32 CHUNK_SIZE = 16;

constexpr u32 WORLD_MAX_CHUNKS_X = 5;
constexpr u32 WORLD_MAX_CHUNKS_Y = 1;
constexpr u32 WORLD_MAX_CHUNKS_Z = 5;

*/

void VoxelChunkArea::Create(f32 radius)
{
    const u32 maxChunksAxis = 2 * Math::Ceil(radius / CHUNK_SIZE); 
    const u32 maxChunks = maxChunksAxis * maxChunksAxis * maxChunksAxis;

    chunks.Resize(maxChunks);
    isOnlyAir.Resize(maxChunks);
    faceCounts.Resize(maxChunks);

    meshData.Resize(maxChunks);

    chunkIndices.Allocate(maxChunksAxis);
    tempIndices.Allocate(maxChunksAxis);

    // A max of 3 faces of the area will be updated
    u32 neededCapacity = maxChunksAxis * maxChunksAxis * 3;
    if (crData.newChunkUpdateList.capacity() < neededCapacity)
    {
        crData.newChunkUpdateList.Reserve(neededCapacity);
        crData.surroundingChunkUpdateList.Reserve(neededCapacity);
    }

    // Allocate chunk data
    for (u32 i = 0; i < maxChunksAxis * maxChunksAxis * maxChunksAxis; i++)
    {
        chunks[i].Allocate(CHUNK_SIZE);
        faceCounts[i] = 0;
        meshData[i] = (VoxelVertex*) PlatformAllocate(4 * maxVoxelFaceCount * sizeof(VoxelVertex));
    }

    areaRadius = radius;
}

void VoxelChunkArea::Free()
{
    // Free chunk data
    for (u32 i = 0; i < chunks.size(); i++)
    {
        chunks[i].Free();
        PlatformFree(meshData[i]);
    }

    chunks.Free();
    isOnlyAir.Free();
    faceCounts.Free();

    meshData.Free();

    chunkIndices.Free();
    tempIndices.Free();
}

inline BlockType GetBlockAt(const VoxelChunkArea& area,
                            u32 chunkX, u32 chunkY, u32 chunkZ,
                            s32 x, s32 y, s32 z)
{
    if (x < 0)
    {
        if (chunkX == 0)
            return BlockType::NONE;

        x = CHUNK_SIZE - 1;
        chunkX--;
    }

    if (x >= CHUNK_SIZE)
    {
        if (chunkX == area.chunkIndices.dimension() - 1)
            return BlockType::NONE;

        x = 0;
        chunkX++;
    }

    if (y < 0)
    {
        if (chunkY == 0)
            return BlockType::NONE;

        y = CHUNK_SIZE - 1;
        chunkY--;
    }

    if (y >= CHUNK_SIZE)
    {
        if (chunkY == area.chunkIndices.dimension() - 1)
            return BlockType::NONE;

        y = 0;
        chunkY++;
    }

    if (z < 0)
    {
        if (chunkZ == 0)
            return BlockType::NONE;

        z = CHUNK_SIZE - 1;
        chunkZ--;
    }

    if (z >= CHUNK_SIZE)
    {
        if (chunkZ == area.chunkIndices.dimension() - 1)
            return BlockType::NONE;

        z = 0;
        chunkZ++;
    }

    const u32 index = area.chunkIndices.at(chunkX, chunkY, chunkZ);
    return area.chunks[index].at(x, y, z);
}

inline f32 GetOcclusion(const VoxelChunkArea& area, VoxelFaceDirection direction, u32 positionIndex,
                        u32 chunkX, u32 chunkY, u32 chunkZ,
                        u32 x, u32 y, u32 z)
{
    // positionIndex -> [0, 7] and direction -> [0, 5]
    u32 offsetIndex = ((u32) direction << 4) | positionIndex;

    u32 chunkIndex = area.chunkIndices.at(chunkX, chunkY, chunkZ);
    const VoxelChunk& chunk = area.chunks[chunkIndex];

    bool side1  = (u32) (GetBlockAt(area, chunkX, chunkY, chunkZ, (s32) x + crData.aoXOffsets[offsetIndex][0], (s32) y + crData.aoYOffsets[offsetIndex][0], (s32) z + crData.aoZOffsets[offsetIndex][0]) != BlockType::NONE);
    bool side2  = (u32) (GetBlockAt(area, chunkX, chunkY, chunkZ, (s32) x + crData.aoXOffsets[offsetIndex][2], (s32) y + crData.aoYOffsets[offsetIndex][2], (s32) z + crData.aoZOffsets[offsetIndex][2]) != BlockType::NONE);
    bool corner = (u32) (GetBlockAt(area, chunkX, chunkY, chunkZ, (s32) x + crData.aoXOffsets[offsetIndex][1], (s32) y + crData.aoYOffsets[offsetIndex][1], (s32) z + crData.aoZOffsets[offsetIndex][1]) != BlockType::NONE);

    if (side1 && side2)
        return 0;

    return (3 - (side1 + side2 + corner)) / 3.0f;
}

void VoxelChunkArea::UpdateChunkMesh(u32 chunkX, u32 chunkY, u32 chunkZ)
{
    f32 halfDim = chunkIndices.dimension() / 2.0f;
    f32 sx = ((f32) chunkX - halfDim);
    f32 sy = ((f32) chunkY - halfDim);
    f32 sz = ((f32) chunkZ - halfDim);

    const Vector3 chunkPosition = areaPosition + Vector3(sx * CHUNK_SIZE, sy * CHUNK_SIZE, sz * CHUNK_SIZE);

    u32 chunkIndex = chunkIndices.at(chunkX, chunkY, chunkZ);
    VoxelChunk& chunk = chunks[chunkIndex];
    u32& faceCount = faceCounts[chunkIndex];
    VoxelVertex*& vertexBuffer = meshData[chunkIndex];
    bool& onlyAir = isOnlyAir[chunkIndex];

    const Vector3 positions[] = {
        Vector3(0.0f, 0.0f, 1.0f),
        Vector3(1.0f, 0.0f, 1.0f),
        Vector3(1.0f, 1.0f, 1.0f),
        Vector3(0.0f, 1.0f, 1.0f),

        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(0.0f, 0.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(1.0f, 1.0f, 0.0f),
    };

    VoxelVertex* voxelVertexPtr = vertexBuffer;
    faceCount = 0;
    onlyAir = true;

    for (u32 z = 0; z < CHUNK_SIZE; z++)
    for (u32 y = 0; y < CHUNK_SIZE; y++)
    for (u32 x = 0; x < CHUNK_SIZE; x++)
    {
        BlockType type = chunk.at(x, y, z);

        if (type == BlockType::NONE)
            continue;
        
        onlyAir = false;

        const Vector3 position = Vector3(x, y, z) + chunkPosition;
        const u32* texIndices = voxelTypeTextureIndices + ((u32) type * 6);

        constexpr f32 texCoordDimension = 1.0f / TEX_PACK_DIMENSION;

        // Add Front Face if needed
        if ((z == CHUNK_SIZE - 1 && (chunkZ < chunkIndices.dimension() - 1 && chunks[chunkIndices.at(chunkX, chunkY, chunkZ + 1)].at(x, y, 0) == BlockType::NONE)) ||
            (z != CHUNK_SIZE - 1 && chunk.at(x, y, z + 1) == BlockType::NONE))
        {
            constexpr VoxelFaceDirection direction = VoxelFaceDirection::FRONT;

            const u32 atlasX = texIndices[(u32) direction] % TEX_PACK_DIMENSION;
            const u32 atlasY = TEX_PACK_DIMENSION - (texIndices[(u32) direction] / TEX_PACK_DIMENSION);
            const Vector4 texCoords = { atlasX * texCoordDimension, atlasY * texCoordDimension, (atlasX + 1) * texCoordDimension, (atlasY - 1) * texCoordDimension };

            f32 a00 = GetOcclusion(*this, direction, 0, chunkX, chunkY, chunkZ, x, y, z);
            f32 a01 = GetOcclusion(*this, direction, 1, chunkX, chunkY, chunkZ, x, y, z);
            f32 a11 = GetOcclusion(*this, direction, 2, chunkX, chunkY, chunkZ, x, y, z);
            f32 a10 = GetOcclusion(*this, direction, 3, chunkX, chunkY, chunkZ, x, y, z);

            VoxelVertex v0 = { positions[0] + position, Vector3::forward, { texCoords.u, texCoords.v }, a00 };
            VoxelVertex v1 = { positions[1] + position, Vector3::forward, { texCoords.s, texCoords.v }, a01 };
            VoxelVertex v2 = { positions[2] + position, Vector3::forward, { texCoords.s, texCoords.t }, a11 };
            VoxelVertex v3 = { positions[3] + position, Vector3::forward, { texCoords.u, texCoords.t }, a10 };

            if (a00 + a11 > a01 + a10)
            {
                v3 = { positions[0] + position, Vector3::forward, { texCoords.u, texCoords.v }, a00 };
                v0 = { positions[1] + position, Vector3::forward, { texCoords.s, texCoords.v }, a01 };
                v1 = { positions[2] + position, Vector3::forward, { texCoords.s, texCoords.t }, a11 };
                v2 = { positions[3] + position, Vector3::forward, { texCoords.u, texCoords.t }, a10 };
            }

            voxelVertexPtr[0] = v0;
            voxelVertexPtr[1] = v1;
            voxelVertexPtr[2] = v2;
            voxelVertexPtr[3] = v3;

            voxelVertexPtr += 4;
            faceCount++;
        }

        // Add Up Face if needed
        if ((y == CHUNK_SIZE - 1 && (chunkY < chunkIndices.dimension() - 1 && chunks[chunkIndices.at(chunkX, chunkY + 1, chunkZ)].at(x, 0, z) == BlockType::NONE)) ||
            (y != CHUNK_SIZE - 1 && chunk.at(x, y + 1, z) == BlockType::NONE))
        {
            constexpr VoxelFaceDirection direction = VoxelFaceDirection::UP;

            const u32 atlasX = texIndices[(u32) direction] % TEX_PACK_DIMENSION;
            const u32 atlasY = TEX_PACK_DIMENSION - (texIndices[(u32) direction] / TEX_PACK_DIMENSION);
            const Vector4 texCoords = { atlasX * texCoordDimension, atlasY * texCoordDimension, (atlasX + 1) * texCoordDimension, (atlasY - 1) * texCoordDimension };

            f32 a00 = GetOcclusion(*this, direction, 3, chunkX, chunkY, chunkZ, x, y, z);
            f32 a01 = GetOcclusion(*this, direction, 2, chunkX, chunkY, chunkZ, x, y, z);
            f32 a11 = GetOcclusion(*this, direction, 7, chunkX, chunkY, chunkZ, x, y, z);
            f32 a10 = GetOcclusion(*this, direction, 6, chunkX, chunkY, chunkZ, x, y, z);

            VoxelVertex v0 = { positions[3] + position, Vector3::up, { texCoords.u, texCoords.v }, a00 };
            VoxelVertex v1 = { positions[2] + position, Vector3::up, { texCoords.s, texCoords.v }, a01 };
            VoxelVertex v2 = { positions[7] + position, Vector3::up, { texCoords.s, texCoords.t }, a11 };
            VoxelVertex v3 = { positions[6] + position, Vector3::up, { texCoords.u, texCoords.t }, a10 };
            
            if (a01 + a10 > a00 + a11)
            {
                v3 = { positions[3] + position, Vector3::up, { texCoords.u, texCoords.v }, a00 };
                v0 = { positions[2] + position, Vector3::up, { texCoords.s, texCoords.v }, a01 };
                v1 = { positions[7] + position, Vector3::up, { texCoords.s, texCoords.t }, a11 };
                v2 = { positions[6] + position, Vector3::up, { texCoords.u, texCoords.t }, a10 };
            }

            voxelVertexPtr[0] = v0;
            voxelVertexPtr[1] = v1;
            voxelVertexPtr[2] = v2;
            voxelVertexPtr[3] = v3;

            voxelVertexPtr += 4;
            faceCount++;
        }

        // Add Right Face if needed
        if ((x == CHUNK_SIZE - 1 && (chunkX < chunkIndices.dimension() - 1 && chunks[chunkIndices.at(chunkX + 1, chunkY, chunkZ)].at(0, y, z) == BlockType::NONE)) ||
            (x != CHUNK_SIZE - 1 && chunk.at(x + 1, y, z) == BlockType::NONE))
        {
            constexpr VoxelFaceDirection direction = VoxelFaceDirection::RIGHT;

            const u32 atlasX = texIndices[(u32) direction] % TEX_PACK_DIMENSION;
            const u32 atlasY = TEX_PACK_DIMENSION - (texIndices[(u32) direction] / TEX_PACK_DIMENSION);
            const Vector4 texCoords = { atlasX * texCoordDimension, atlasY * texCoordDimension, (atlasX + 1) * texCoordDimension, (atlasY - 1) * texCoordDimension };

            f32 a00 = GetOcclusion(*this, direction, 7, chunkX, chunkY, chunkZ, x, y, z);
            f32 a01 = GetOcclusion(*this, direction, 2, chunkX, chunkY, chunkZ, x, y, z);
            f32 a11 = GetOcclusion(*this, direction, 1, chunkX, chunkY, chunkZ, x, y, z);
            f32 a10 = GetOcclusion(*this, direction, 4, chunkX, chunkY, chunkZ, x, y, z);

            VoxelVertex v0 = { positions[7] + position, Vector3::right, { texCoords.u, texCoords.t }, a00 };
            VoxelVertex v1 = { positions[2] + position, Vector3::right, { texCoords.s, texCoords.t }, a01 };
            VoxelVertex v2 = { positions[1] + position, Vector3::right, { texCoords.s, texCoords.v }, a11 };
            VoxelVertex v3 = { positions[4] + position, Vector3::right, { texCoords.u, texCoords.v }, a10 };

            if (a01 + a10 > a00 + a11)
            {
                v3 = { positions[7] + position, Vector3::right, { texCoords.u, texCoords.t }, a00 };
                v0 = { positions[2] + position, Vector3::right, { texCoords.s, texCoords.t }, a01 };
                v1 = { positions[1] + position, Vector3::right, { texCoords.s, texCoords.v }, a11 };
                v2 = { positions[4] + position, Vector3::right, { texCoords.u, texCoords.v }, a10 };
            }

            voxelVertexPtr[0] = v0;
            voxelVertexPtr[1] = v1;
            voxelVertexPtr[2] = v2;
            voxelVertexPtr[3] = v3;

            voxelVertexPtr += 4;
            faceCount++;
        }

        // Add Left Face if needed
        if ((x == 0 && (chunkX > 0 && chunks[chunkIndices.at(chunkX - 1, chunkY, chunkZ)].at(CHUNK_SIZE - 1, y, z) == BlockType::NONE)) ||
            (x != 0 && chunk.at(x - 1, y, z) == BlockType::NONE))
        {
            constexpr VoxelFaceDirection direction = VoxelFaceDirection::LEFT;

            const u32 atlasX = texIndices[(u32) direction] % TEX_PACK_DIMENSION;
            const u32 atlasY = TEX_PACK_DIMENSION - (texIndices[(u32) direction] / TEX_PACK_DIMENSION);
            const Vector4 texCoords = { atlasX * texCoordDimension, atlasY * texCoordDimension, (atlasX + 1) * texCoordDimension, (atlasY - 1) * texCoordDimension };

            f32 a00 = GetOcclusion(*this, direction, 3, chunkX, chunkY, chunkZ, x, y, z);
            f32 a01 = GetOcclusion(*this, direction, 6, chunkX, chunkY, chunkZ, x, y, z);
            f32 a11 = GetOcclusion(*this, direction, 5, chunkX, chunkY, chunkZ, x, y, z);
            f32 a10 = GetOcclusion(*this, direction, 0, chunkX, chunkY, chunkZ, x, y, z);

            VoxelVertex v0 = { positions[3] + position, Vector3::left, { texCoords.u, texCoords.t }, a00 };
            VoxelVertex v1 = { positions[6] + position, Vector3::left, { texCoords.s, texCoords.t }, a01 };
            VoxelVertex v2 = { positions[5] + position, Vector3::left, { texCoords.s, texCoords.v }, a11 };
            VoxelVertex v3 = { positions[0] + position, Vector3::left, { texCoords.u, texCoords.v }, a10 };

            if (a01 + a10 > a00 + a11)
            {
                v3 = { positions[3] + position, Vector3::left, { texCoords.u, texCoords.t }, a00 };
                v0 = { positions[6] + position, Vector3::left, { texCoords.s, texCoords.t }, a01 };
                v1 = { positions[5] + position, Vector3::left, { texCoords.s, texCoords.v }, a11 };
                v2 = { positions[0] + position, Vector3::left, { texCoords.u, texCoords.v }, a10 };
            }

            voxelVertexPtr[0] = v0;
            voxelVertexPtr[1] = v1;
            voxelVertexPtr[2] = v2;
            voxelVertexPtr[3] = v3;

            voxelVertexPtr += 4;
            faceCount++;
        }

        // Add Down Face if needed
        if ((y == 0 && (chunkY > 0 && chunks[chunkIndices.at(chunkX, chunkY - 1, chunkZ)].at(x, CHUNK_SIZE - 1, z) == BlockType::NONE)) ||
            (y != 0 && chunk.at(x, y - 1, z) == BlockType::NONE))
        {
            constexpr VoxelFaceDirection direction = VoxelFaceDirection::DOWN;

            const u32 atlasX = texIndices[(u32) direction] % TEX_PACK_DIMENSION;
            const u32 atlasY = TEX_PACK_DIMENSION - (texIndices[(u32) direction] / TEX_PACK_DIMENSION);
            const Vector4 texCoords = { atlasX * texCoordDimension, atlasY * texCoordDimension, (atlasX + 1) * texCoordDimension, (atlasY - 1) * texCoordDimension };

            f32 a00 = GetOcclusion(*this, direction, 1, chunkX, chunkY, chunkZ, x, y, z);
            f32 a01 = GetOcclusion(*this, direction, 0, chunkX, chunkY, chunkZ, x, y, z);
            f32 a11 = GetOcclusion(*this, direction, 5, chunkX, chunkY, chunkZ, x, y, z);
            f32 a10 = GetOcclusion(*this, direction, 4, chunkX, chunkY, chunkZ, x, y, z);

            VoxelVertex v0 = { positions[1] + position, Vector3::down, { texCoords.u, texCoords.v }, a00 };
            VoxelVertex v1 = { positions[0] + position, Vector3::down, { texCoords.s, texCoords.v }, a01 };
            VoxelVertex v2 = { positions[5] + position, Vector3::down, { texCoords.s, texCoords.t }, a11 };
            VoxelVertex v3 = { positions[4] + position, Vector3::down, { texCoords.u, texCoords.t }, a10 };

            if (a01 + a10 > a00 + a11)
            {
                v3 = { positions[1] + position, Vector3::down, { texCoords.u, texCoords.v }, a00 };
                v0 = { positions[0] + position, Vector3::down, { texCoords.s, texCoords.v }, a01 };
                v1 = { positions[5] + position, Vector3::down, { texCoords.s, texCoords.t }, a11 };
                v2 = { positions[4] + position, Vector3::down, { texCoords.u, texCoords.t }, a10 };
            }

            voxelVertexPtr[0] = v0;
            voxelVertexPtr[1] = v1;
            voxelVertexPtr[2] = v2;
            voxelVertexPtr[3] = v3;

            voxelVertexPtr += 4;
            faceCount++;
        }

        // Add Back Face if needed
        if ((z == 0 && (chunkZ > 0 && chunks[chunkIndices.at(chunkX, chunkY, chunkZ - 1)].at(x, y, CHUNK_SIZE - 1) == BlockType::NONE)) ||
            (z != 0 && chunk.at(x, y, z - 1) == BlockType::NONE))
        {
            constexpr VoxelFaceDirection direction = VoxelFaceDirection::BACK;

            const u32 atlasX = texIndices[(u32) direction] % TEX_PACK_DIMENSION;
            const u32 atlasY = TEX_PACK_DIMENSION - (texIndices[(u32) direction] / TEX_PACK_DIMENSION);
            const Vector4 texCoords = { atlasX * texCoordDimension, atlasY * texCoordDimension, (atlasX + 1) * texCoordDimension, (atlasY - 1) * texCoordDimension };

            f32 a00 = GetOcclusion(*this, direction, 6, chunkX, chunkY, chunkZ, x, y, z);
            f32 a01 = GetOcclusion(*this, direction, 7, chunkX, chunkY, chunkZ, x, y, z);
            f32 a11 = GetOcclusion(*this, direction, 4, chunkX, chunkY, chunkZ, x, y, z);
            f32 a10 = GetOcclusion(*this, direction, 5, chunkX, chunkY, chunkZ, x, y, z);

            VoxelVertex v0 = { positions[6] + position, Vector3::back, { texCoords.u, texCoords.t }, a00 };
            VoxelVertex v1 = { positions[7] + position, Vector3::back, { texCoords.s, texCoords.t }, a01 };
            VoxelVertex v2 = { positions[4] + position, Vector3::back, { texCoords.s, texCoords.v }, a11 };
            VoxelVertex v3 = { positions[5] + position, Vector3::back, { texCoords.u, texCoords.v }, a10 };

            if (a01 + a10 > a00 + a11)
            {
                v3 = { positions[6] + position, Vector3::back, { texCoords.u, texCoords.t }, a00 };
                v0 = { positions[7] + position, Vector3::back, { texCoords.s, texCoords.t }, a01 };
                v1 = { positions[4] + position, Vector3::back, { texCoords.s, texCoords.v }, a11 };
                v2 = { positions[5] + position, Vector3::back, { texCoords.u, texCoords.v }, a10 };
            }

            voxelVertexPtr[0] = v0;
            voxelVertexPtr[1] = v1;
            voxelVertexPtr[2] = v2;
            voxelVertexPtr[3] = v3;

            voxelVertexPtr += 4;
            faceCount++;
        }
    }
}

template<typename T>
static inline void AddToArrayIfNotPresent(DynamicArray<T>& array, const T& value)
{
    for (int i = 0; i < array.size(); i++)
    {
        if (array[i] == value)
            return;
    }

    array.EmplaceBack(value);
}

constexpr f32 maxHeightAmplitude = 16.0f;
static inline f32 GetHeightAtPosition(const SimplexNoise& noise, f32 x, f32 z)
{
    constexpr f32 mult = 0.0078125f;
    return maxHeightAmplitude * noise.fractal(4, x * mult, z * mult);
}

void VoxelChunkArea::InitializeChunkArea(const SimplexNoise& noise, const Vector3& position)
{
    const s32 px = position.x / CHUNK_SIZE;
    const s32 py = position.y / CHUNK_SIZE;
    const s32 pz = position.z / CHUNK_SIZE;

    u32 index = 0;

    for (u32 z = 0; z < chunkIndices.dimension(); z++)
    for (u32 y = 0; y < chunkIndices.dimension(); y++)
    for (u32 x = 0; x < chunkIndices.dimension(); x++)
    {
        VoxelChunk& chunk = chunks[index];
        chunkIndices.at(x, y, z) = index;

        // Offsets for chunk
        f32 sx = ((f32) x - (chunkIndices.dimension() / 2.0f));
        f32 sy = ((f32) y - (chunkIndices.dimension() / 2.0f));
        f32 sz = ((f32) z - (chunkIndices.dimension() / 2.0f));

        const Vector3 chunkPosition = Vector3(sx * CHUNK_SIZE, sy * CHUNK_SIZE, sz * CHUNK_SIZE);
        const Vector3 worldPosition = areaPosition + chunkPosition;

        bool& onlyAir = isOnlyAir[index];
        onlyAir = true;

        if (worldPosition.y - (CHUNK_SIZE / 2.0f) <= maxHeightAmplitude)
        {
            for (u32 cz = 0; cz < CHUNK_SIZE; cz++)
            for (u32 cx = 0; cx < CHUNK_SIZE; cx++)
            {
                const f32 fx = cx + worldPosition.x;
                const f32 fz = cz + worldPosition.z;
                f32 height = GetHeightAtPosition(noise, fx, fz);
                
                for (u32 cy = 0; cy < CHUNK_SIZE; cy++)
                {
                    const f32 fy = (cy + worldPosition.y);
                    
                    if (fy <= height)
                        chunk.at(cx, cy, cz) = (height - fy >= 1.0f) ? ((height - fy >= 4.0f) ? BlockType::STONE : BlockType::DIRT) : BlockType::GRASS;
                    else
                        chunk.at(cx, cy, cz) = BlockType::NONE;

                    onlyAir = onlyAir && (fy > height);
                }
            }
        }

        index++;
    }

    for (u32 z = 0; z < chunkIndices.dimension(); z++)
    for (u32 y = 0; y < chunkIndices.dimension(); y++)
    for (u32 x = 0; x < chunkIndices.dimension(); x++)
    {
        if (isOnlyAir[chunkIndices.at(x, y, z)])
            continue;

        UpdateChunkMesh(x, y, z);
    }

    areaPosition = Vector3((f32) px * CHUNK_SIZE, (f32) py * CHUNK_SIZE, (f32) pz * CHUNK_SIZE);

    newUpdatesLeft = 0;
    surrUpdatesLeft = 0;
}

// TODO: Updating chunk meshes is very slow. Maybe move it to a different thread?
//       For now this function updates chunk meshes across multiple frames to maintain framerate.
//       This works really well and can be a backup if someone doesn't want multithreading for some reason.
void VoxelChunkArea::UpdateChunkArea(const SimplexNoise& noise, const Vector3& position)
{
    if (surrUpdatesLeft > 0)
    {
        // Only allow a limited number of chunk meshes to be generated in each frame
        // Also no furthur updates are to be made till the current batch of updates is not finished
        s32 start = Min(chunkUpdatesPerFrame * surrUpdatesLeft - 1, (u32) crData.surroundingChunkUpdateList.size() - 1);
        s32 end = chunkUpdatesPerFrame * (surrUpdatesLeft - 1);

        // Update newly added chunk meshes
        for (int i = start; i >= end; i--)
        {
            const ChunkUpdateData& data = crData.surroundingChunkUpdateList[i];

            if (isOnlyAir[data.index])
                continue;

            UpdateChunkMesh(data.x, data.y, data.z);
        }

        surrUpdatesLeft--;
        return;
    }

    if (newUpdatesLeft > 0)
    {
        // Only allow a limited number of chunk meshes to be generated in each frame
        // Also no furthur updates are to be made till the current batch of updates is not finished
        s32 start = Min(chunkUpdatesPerFrame * newUpdatesLeft - 1, (u32) crData.newChunkUpdateList.size() - 1);
        s32 end = chunkUpdatesPerFrame * (newUpdatesLeft - 1);

        // Update newly added chunk meshes
        for (int i = start; i >= end; i--)
        {
            const ChunkUpdateData& data = crData.newChunkUpdateList[i];

            if (isOnlyAir[data.index])
                continue;

            UpdateChunkMesh(data.x, data.y, data.z);
        }

        newUpdatesLeft--;
        return;
    }

    // Only update area if player moves from one chunk to another
    const s32 px = position.x / CHUNK_SIZE;
    const s32 py = position.y / CHUNK_SIZE;
    const s32 pz = position.z / CHUNK_SIZE;

    {   // If player is in same chunk no need to update
        const s32 ax = areaPosition.x / CHUNK_SIZE;
        const s32 ay = areaPosition.y / CHUNK_SIZE;
        const s32 az = areaPosition.z / CHUNK_SIZE;

        if (px == ax && py == ay && pz == az)
            return;
    }

    // This is the position of the center chunk
    Vector3 prevAreaPosition = areaPosition;
    areaPosition = Vector3((f32) px * CHUNK_SIZE, (f32) py * CHUNK_SIZE, (f32) pz * CHUNK_SIZE);

    Vector3 displacement = (areaPosition - prevAreaPosition) / Vector3(CHUNK_SIZE); 

    {   // Just to be safe, only work with 1 unit displacements in all directions
        displacement.x = Clamp(displacement.x, -1.0f, 1.0f);
        displacement.y = Clamp(displacement.y, -1.0f, 1.0f);
        displacement.z = Clamp(displacement.z, -1.0f, 1.0f);
        
        areaPosition = prevAreaPosition + displacement * Vector3(CHUNK_SIZE);
    }

    crData.surroundingChunkUpdateList.Clear(false);
    crData.newChunkUpdateList.Clear(false);

    {   // Determine which indices must be updated
        s32 changeX = !Math::AlmostEquals(displacement.x, 0.0f) ? ((displacement.x > 0.0f) ? (s32) chunkIndices.dimension() - 1 : 0) : -1;
        s32 surrX   = !Math::AlmostEquals(displacement.x, 0.0f) ? ((displacement.x > 0.0f) ? (s32) chunkIndices.dimension() - 2 : 1) : -1;

        s32 changeY = !Math::AlmostEquals(displacement.y, 0.0f) ? ((displacement.y > 0.0f) ? (s32) chunkIndices.dimension() - 1 : 0) : -1;
        s32 surrY   = !Math::AlmostEquals(displacement.y, 0.0f) ? ((displacement.y > 0.0f) ? (s32) chunkIndices.dimension() - 2 : 1) : -1;

        s32 changeZ = !Math::AlmostEquals(displacement.z, 0.0f) ? ((displacement.z > 0.0f) ? (s32) chunkIndices.dimension() - 1 : 0) : -1;
        s32 surrZ   = !Math::AlmostEquals(displacement.z, 0.0f) ? ((displacement.z > 0.0f) ? (s32) chunkIndices.dimension() - 2 : 1) : -1;
        
        for (s32 z = 0; z < chunkIndices.dimension(); z++)
        for (s32 y = 0; y < chunkIndices.dimension(); y++)
        for (s32 x = 0; x < chunkIndices.dimension(); x++)
        {
            s32 xi = Wrap(x - (s32) displacement.x, 0, (s32) chunkIndices.dimension());
            s32 yi = Wrap(y - (s32) displacement.y, 0, (s32) chunkIndices.dimension());
            s32 zi = Wrap(z - (s32) displacement.z, 0, (s32) chunkIndices.dimension());

            tempIndices.at(xi, yi, zi) = chunkIndices.at(x, y, z);

            if (xi == changeX || yi == changeY || zi == changeZ ||
                xi == surrX   || yi == surrY   || zi == surrZ)
            {
                ChunkUpdateData data;
                data.index = chunkIndices.at(x, y, z);

                data.x = xi;
                data.y = yi;
                data.z = zi;

                if (xi == surrX   || yi == surrY   || zi == surrZ)
                    AddToArrayIfNotPresent(crData.surroundingChunkUpdateList, data);
                else
                    AddToArrayIfNotPresent(crData.newChunkUpdateList, data);
            }
        }

        {   // Swap index arrays
            chunkIndices.SwapWith(tempIndices);
        }
    }

    {   // Update relevant chunk data
        for (int i = 0; i < crData.newChunkUpdateList.size(); i++)
        {
            ChunkUpdateData& data = crData.newChunkUpdateList[i];

            // Offsets for chunk
            f32 sx = ((f32) data.x - (chunkIndices.dimension() / 2.0f));
            f32 sy = ((f32) data.y - (chunkIndices.dimension() / 2.0f));
            f32 sz = ((f32) data.z - (chunkIndices.dimension() / 2.0f));

            const Vector3 chunkPosition = Vector3(sx * CHUNK_SIZE, sy * CHUNK_SIZE, sz * CHUNK_SIZE);
            const Vector3 worldPosition = areaPosition + chunkPosition;

            bool& onlyAir = isOnlyAir[data.index];
            onlyAir = true;

            if (worldPosition.y - (CHUNK_SIZE / 2.0f) <= maxHeightAmplitude)
            {
                for (u32 cz = 0; cz < CHUNK_SIZE; cz++)
                for (u32 cx = 0; cx < CHUNK_SIZE; cx++)
                {
                    const f32 fx = cx + worldPosition.x;
                    const f32 fz = cz + worldPosition.z;
                    f32 height = GetHeightAtPosition(noise, fx, fz);

                    for (u32 cy = 0; cy < CHUNK_SIZE; cy++)
                    {
                        const f32 fy = (cy + worldPosition.y);

                        if (fy <= height)
                            chunks[data.index].at(cx, cy, cz) = (height - fy >= 1.0f) ? ((height - fy >= 4.0f) ? BlockType::STONE : BlockType::DIRT) : BlockType::GRASS;
                        else
                            chunks[data.index].at(cx, cy, cz) = BlockType::NONE;

                        onlyAir = onlyAir && (fy > height);
                    }
                }
            }
        }

        newUpdatesLeft = (crData.newChunkUpdateList.size() - 1) / chunkUpdatesPerFrame;
        surrUpdatesLeft = (crData.surroundingChunkUpdateList.size() - 1) / chunkUpdatesPerFrame;
    }
}

namespace ChunkRenderer
{

void Init()
{
    // Set up common vertex array
    glGenVertexArrays(1, &crData.vao);
    glBindVertexArray(crData.vao);

    glGenBuffers(1, &crData.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, crData.vbo);
    glBufferData(GL_ARRAY_BUFFER, maxChunkBatchSize, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(VoxelVertex), (const void*) offsetof(VoxelVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(VoxelVertex), (const void*) offsetof(VoxelVertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(VoxelVertex), (const void*) offsetof(VoxelVertex, texCoord));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(VoxelVertex), (const void*) offsetof(VoxelVertex, occlusion));

    // Set up common index buffer
    u32* indices = (u32*) PlatformAllocate(12 * maxFacesInBatch * sizeof(u32));
    AssertWithMessage(indices != nullptr, "Couldn't allocate index buffer data for chunks!");

    {   // Mesh indices
        u32 offset = 0;
        for (int i = 0; i < 6 * maxFacesInBatch; i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
            offset += 4;
        }

        glGenBuffers(1, &crData.iboMesh);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, crData.iboMesh);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * maxFacesInBatch * sizeof(u32), indices, GL_STATIC_DRAW);
    }

    {   // Wireframe Indices
        u32 offset = 0;
        for (int i = 0; i < 12 * maxFacesInBatch; i += 12)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;

            indices[i + 2] = offset + 1;
            indices[i + 3] = offset + 2;

            indices[i + 4] = offset + 2;
            indices[i + 5] = offset + 0;

            indices[i + 6] = offset + 2;
            indices[i + 7] = offset + 3;

            indices[i + 8] = offset + 3;
            indices[i + 9] = offset + 0;

            indices[i + 10] = offset + 0;
            indices[i + 11] = offset + 2;

            offset += 4;
        }
        
        glGenBuffers(1, &crData.iboWireframe);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, crData.iboWireframe);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * maxFacesInBatch * sizeof(u32), indices, GL_STATIC_DRAW);
    }

    PlatformFree(indices);

    // Ambient Occlusion
    FillOcclusionOffsetTables(crData.aoXOffsets, crData.aoYOffsets, crData.aoZOffsets);
}

void Shutdown()
{
    // Nothing to delete other than GPU stuff which gets deleted anyways
}

void Begin(Camera& camera, const Texture& atlas)
{
    crData.camera = &camera;
    camera.UpdateViewFrustum();

    atlas.Bind(ATLAS_BIND_SLOT);
}

void End()
{
    crData.camera = nullptr;    // No need to do this...
}

// Simple AABB in Frustum check
bool IsChunkInFrustum(const Vector3& bottomLeftBack, const Vector3& topRightFront)
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = crData.camera->viewFrustum().planes[i];

        int inCount = 8;

        {   // Bottom Left Back
            Vector3 point = bottomLeftBack;
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        {   // Bottom Right Back
            Vector3 point = Vector3(topRightFront.x, bottomLeftBack.y, bottomLeftBack.z);
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        {   // Top Right Back
            Vector3 point = Vector3(topRightFront.x, topRightFront.y, bottomLeftBack.z);
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        {   // Top Left Back
            Vector3 point = Vector3(bottomLeftBack.x, topRightFront.y, bottomLeftBack.z);
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }
        
        {   // Bottom Left Front
            Vector3 point = Vector3(bottomLeftBack.x, topRightFront.y, topRightFront.z);
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        {   // Bottom Right Front
            Vector3 point = Vector3(topRightFront.x, bottomLeftBack.y, topRightFront.z);
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        {   // Top Right Front
            Vector3 point = topRightFront;
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        {   // Top Left Front
            Vector3 point = Vector3(bottomLeftBack.x, topRightFront.y, topRightFront.z);
            inCount -= (plane.EvaluatePoint(point) < 0.0f);
        }

        if (inCount <= 0)
            return false;
    }

    return true;
}

static void FlushBatch(Shader& shader, u64& batchSize, u32& batchFaceCount, DebugRendererStats& stats, const DebugRendererSettings& settings)
{
    if (settings.showBatches)
    {
        const Vector3 colors[] = {
            Vector3(1.0f, 0.0f, 0.0f),
            Vector3(0.0f, 1.0f, 0.0f),
            Vector3(0.0f, 0.0f, 1.0f),
            Vector3(1.0f, 1.0f, 0.0f),
            Vector3(0.0f, 1.0f, 1.0f),
            Vector3(1.0f, 0.0f, 1.0f),
            Vector3(1.0f, 0.0f, 1.0f),
        };

        const Vector3 color = colors[stats.batches % 7];
        shader.SetUniform4f("u_color", color.r, color.g, color.b, 1.0f);
    }

    if (settings.showWireframe)
        glDrawElements(GL_LINES, 12 * batchFaceCount, GL_UNSIGNED_INT, nullptr);
    else
        glDrawElements(GL_TRIANGLES, 6 * batchFaceCount, GL_UNSIGNED_INT, nullptr);

    stats.trianglesRendered += 2 * batchFaceCount;
    stats.batches++;
    
    batchFaceCount = 0;
    batchSize = 0;
}

void RenderChunkArea(VoxelChunkArea& area, Shader& shader, DebugRendererStats& stats, const DebugRendererSettings& settings)
{
    AssertWithMessage(crData.camera != nullptr, "ChunkRenderer::Begin() not called!");

    shader.Bind();
    shader.SetUniformMatrix4("u_viewProjection", crData.camera->viewProjection());
    shader.SetUniform1i("u_texture", ATLAS_BIND_SLOT);

    if (!settings.showBatches)
        shader.SetUniform4f("u_color", 1.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(crData.vao);
    glBindBuffer(GL_ARRAY_BUFFER, crData.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, settings.showWireframe ? crData.iboWireframe : crData.iboMesh);

    stats.trianglesRendered = 0;
    stats.batches = 0;

    u64 batchSize = 0;
    u32 batchFaceCount = 0;

    for (u32 z = 0; z < area.chunkIndices.dimension(); z++)
    for (u32 y = 0; y < area.chunkIndices.dimension(); y++)
    for (u32 x = 0; x < area.chunkIndices.dimension(); x++)
    {
        u32 index = area.chunkIndices.at(x, y, z);

        if (area.isOnlyAir[index] || area.faceCounts[index] == 0)
            continue;

        {   // Check for frustum culling
            // Offsets for chunk
            f32 sx = ((f32) x - (area.chunkIndices.dimension() / 2.0f));
            f32 sy = ((f32) y - (area.chunkIndices.dimension() / 2.0f));
            f32 sz = ((f32) z - (area.chunkIndices.dimension() / 2.0f));

            const Vector3 chunkPosition = area.areaPosition + Vector3(sx * CHUNK_SIZE, sy * CHUNK_SIZE, sz * CHUNK_SIZE);

            Vector3 topRightFront = chunkPosition + Vector3(CHUNK_SIZE + 1);
            if (!IsChunkInFrustum(chunkPosition, topRightFront))
                continue;
        }

        u64 dataSize = 4 * area.faceCounts[index] * sizeof(VoxelVertex);

        if (batchSize + dataSize >= maxChunkBatchSize)
            FlushBatch(shader, batchSize, batchFaceCount, stats, settings);

        // Turns out, batching on the GPU directly is slightly faster than batching on CPU and sending it to GPU
        glBufferSubData(GL_ARRAY_BUFFER, batchSize, dataSize, area.meshData[index]);
        batchFaceCount += area.faceCounts[index];
        batchSize += dataSize;
    }

    if (batchSize > 0)
        FlushBatch(shader, batchSize, batchFaceCount, stats, settings);
}

} // namespace ChunkRenderer