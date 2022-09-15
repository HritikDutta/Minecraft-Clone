#pragma once

#include "core/types.h"

constexpr u32 TEX_PACK_DIMENSION = 16;
constexpr s32 ATLAS_BIND_SLOT = 0;

#define UV(x, y) (y * 16 + x)

// Indices are in the order: Front, Up, Right, Left, Down, Back
const u32 voxelTypeTextureIndices[] = {
    // Air (Not used in rendering)
    UV( 0,  0), UV( 0,  0), UV( 0,  0), UV( 0,  0), UV( 0,  0), UV( 0,  0),

    // Glass
    UV( 1,  3), UV( 1,  3), UV( 1,  3), UV( 1,  3), UV( 1,  3), UV( 1,  3),

    // Dirt
    UV( 2,  0), UV( 2,  0), UV( 2,  0), UV( 2,  0), UV( 2,  0), UV( 2,  0),

    // Grass
    UV( 3,  0), UV( 2,  9), UV( 3,  0), UV( 3,  0), UV( 2,  0), UV( 3,  0),

    // Sand
    UV( 2,  1), UV( 2,  1), UV( 2,  1), UV( 2,  1), UV( 2,  1), UV( 2,  1),
    
    // Sand Stone
    UV( 0, 12), UV( 0, 11), UV( 0, 12), UV( 0, 12), UV( 0, 13), UV( 0, 12),
    
    // Gravel
    UV( 3,  1), UV( 3,  1), UV( 3,  1), UV( 3,  1), UV( 3,  1), UV( 3,  1),

    // Wood
    UV( 4,  1), UV( 5,  1), UV( 4,  1), UV( 4,  1), UV( 5,  1), UV( 4,  1),
    
    // Wooden Plank
    UV( 4,  0), UV( 4,  0), UV( 4,  0), UV( 4,  0), UV( 4,  0), UV( 4,  0),
    
    // Clay
    UV( 8,  4), UV( 8,  4), UV( 8,  4), UV( 8,  4), UV( 8,  4), UV( 8,  4),
    
    // Bricks
    UV( 7,  0), UV( 7,  0), UV( 7,  0), UV( 7,  0), UV( 7,  0), UV( 7,  0),

    // Cobble Stone
    UV( 0,  1), UV( 0,  1), UV( 0,  1), UV( 0,  1), UV( 0,  1), UV( 0,  1),
    
    // Mossy Cobble Stone
    UV( 4,  2), UV( 4,  2), UV( 4,  2), UV( 4,  2), UV( 4,  2), UV( 4,  2),

    // Stone
    UV( 1,  0), UV( 1,  0), UV( 1,  0), UV( 1,  0), UV( 1,  0), UV( 1,  0),

    // Bedrock
    UV( 1,  1), UV( 1,  1), UV( 1,  1), UV( 1,  1), UV( 1,  1), UV( 1,  1),

    // Obsidian
    UV( 5,  2), UV( 5,  2), UV( 5,  2), UV( 5,  2), UV( 5,  2), UV( 5,  2),
};