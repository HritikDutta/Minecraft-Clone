#pragma once

#include "core/types.h"

constexpr u32 TEX_PACK_DIMENSION = 16;
constexpr int ATLAS_BIND_SLOT = 0;

// Indices are in the order: Front, Up, Right, Left, Down, Back
const u32 voxelTypeTextureIndices[] = {
    // Air (Not used in rendering)
    0, 0, 0, 0, 0, 0,

    // Dirt
    2, 2, 2, 2, 2, 2,

    // Sand
    18, 18, 18, 18, 18, 18,

    // Stone
    1, 1, 1, 1, 1, 1,

    // Grass
    3, 146, 3, 3, 2, 3,

    // Bedrock
    17, 17, 17, 17, 17, 17,
};