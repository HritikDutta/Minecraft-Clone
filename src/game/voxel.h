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

    DIRT,
    SAND,
    STONE,
    GRASS,
    BEDROCK,

    NUM_TYPES
};

constexpr char* blockTypeNames[] = {
    "None",
    "Dirt",
    "Sand",
    "Stone",
    "Grass",
    "Bedrock"
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