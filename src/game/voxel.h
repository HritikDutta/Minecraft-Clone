#pragma once

#include "math/math.h"

// TODO: Make a proper Vector3Int in math library
struct Vector3Int
{
    s32 x, y, z;

    Vector3Int operator+(const Vector3Int& rhs) const
    {
        return { x + rhs.x, y + rhs.y, z + rhs.z };
    }
};

enum struct BlockType : u8
{
    NONE,   // Not Rendered

    // Opaque Blocks
    DIRT,
    GRASS,
    SAND,
    SAND_STONE,
    GRAVEL,
    WOOD,
    WOODEN_PLANK,
    CLAY,
    BRICKS,
    COBBLE_STONE,
    MOSSY_COBBLE_STONE,
    STONE,
    BEDROCK,
    OBSIDIAN,

    NUM_TYPES
};

constexpr char* blockTypeNames[] = {
    "None",
    "Dirt",
    "Grass",
    "Sand",
    "Sand Stone",
    "Gravel",
    "Wood",
    "Wooden Plank",
    "Clay",
    "Bricks",
    "Cobble Stone",
    "Mossy Cobble Stone",
    "Stone",
    "Bedrock",
    "Obsidian",
};

enum struct VoxelFaceDirection : u32
{
    FRONT,
    UP,
    RIGHT,
    LEFT,
    DOWN,
    BACK
};

inline bool VoxelBlockHasTransparency(BlockType type)
{
    const u8 typeAsU8 = (u8) type;
    return typeAsU8 >= (u8) BlockType::NONE && typeAsU8 < (u8) BlockType::DIRT;
}