#include "chunk_area.h"
#include "chunk_renderer.h"

void CorrectBlockIndex(Vector3Int& chunkIndex, Vector3Int& blockIndex)
{
    if (blockIndex.x < 0)
    {
        blockIndex.x = CHUNK_SIZE - 1;
        chunkIndex.x--;
    }

    if (blockIndex.x > CHUNK_SIZE - 1)
    {
        blockIndex.x = 0;
        chunkIndex.x++;
    }

    if (blockIndex.y < 0)
    {
        blockIndex.y = CHUNK_SIZE - 1;
        chunkIndex.y--;
    }

    if (blockIndex.y > CHUNK_SIZE - 1)
    {
        blockIndex.y = 0;
        chunkIndex.y++;
    }

    if (blockIndex.z < 0)
    {
        blockIndex.z = CHUNK_SIZE - 1;
        chunkIndex.z--;
    }

    if (blockIndex.z > CHUNK_SIZE - 1)
    {
        blockIndex.z = 0;
        chunkIndex.z++;
    }
};

void PlaceBlockAtPosition(VoxelChunkArea& area, const Vector3Int& chunkIndex, const Vector3Int& blockIndex, BlockType blockType)
{
    u32 index = area.chunkIndices.at(chunkIndex.x, chunkIndex.y, chunkIndex.z);
    area.chunks[index].at(blockIndex.x, blockIndex.y, blockIndex.z) = blockType;
    area.UpdateChunkMesh(chunkIndex.x, chunkIndex.y, chunkIndex.z);

    {   // Update neighbouring chunk meshs if block is at any edge

        if (blockIndex.x == 0)
            area.UpdateChunkMesh(chunkIndex.x - 1, chunkIndex.y, chunkIndex.z);
        if (blockIndex.x == CHUNK_SIZE - 1)
            area.UpdateChunkMesh(chunkIndex.x + 1, chunkIndex.y, chunkIndex.z);

        if (blockIndex.y == 0)
            area.UpdateChunkMesh(chunkIndex.x, chunkIndex.y - 1, chunkIndex.z);
        if (blockIndex.y == CHUNK_SIZE - 1)
            area.UpdateChunkMesh(chunkIndex.x, chunkIndex.y + 1, chunkIndex.z);

        if (blockIndex.z == 0)
            area.UpdateChunkMesh(chunkIndex.x, chunkIndex.y, chunkIndex.z - 1);
        if (blockIndex.z == CHUNK_SIZE - 1)
            area.UpdateChunkMesh(chunkIndex.x, chunkIndex.y, chunkIndex.z + 1);
            
    }
};