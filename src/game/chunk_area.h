#pragma once

#include "chunk_renderer.h"

void CorrectBlockIndex(Vector3Int& chunkIndex, Vector3Int& blockIndex);

void PlaceBlockAtPosition(VoxelChunkArea& area, const Vector3Int& chunkIndex, const Vector3Int& blockIndex, BlockType blockType);