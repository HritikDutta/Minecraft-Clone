#pragma once

#include "core/types.h"
#include "voxel.h"

inline void FillOcclusionOffsetTables(s32 xoffsets[][3], s32 yoffsets[][3], s32 zoffsets[][3])
{
    {   // Front
        {
            constexpr u32 index = ((u32) VoxelFaceDirection::FRONT << 4) | 0;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] =  0;

            yoffsets[index][0] =  0;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::FRONT << 4) | 1;
            xoffsets[index][0] = 0;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] =  0;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::FRONT << 4) | 2;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 0;

            yoffsets[index][0] = 0;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::FRONT << 4) | 3;
            xoffsets[index][0] =  0;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 0;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }
    }

    {   // Up
        {
            constexpr u32 index = ((u32) VoxelFaceDirection::UP << 4) | 3;
            xoffsets[index][0] =  0;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 0;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::UP << 4) | 2;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 0;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = 0;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::UP << 4) | 7;
            xoffsets[index][0] = 0;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] =  0;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::UP << 4) | 6;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] =  0;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] =  0;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }
    }

    {   // Right
        {
            constexpr u32 index = ((u32) VoxelFaceDirection::RIGHT << 4) | 7;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 0;

            zoffsets[index][0] =  0;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::RIGHT << 4) | 2;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = 0;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 0;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::RIGHT << 4) | 1;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] =  0;

            zoffsets[index][0] = 0;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::RIGHT << 4) | 4;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] =  0;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] =  0;
        }
    }

    {   // Left
        {
            constexpr u32 index = ((u32) VoxelFaceDirection::LEFT << 4) | 3;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 0;

            zoffsets[index][0] = 0;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::LEFT << 4) | 6;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = 0;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] =  0;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::LEFT << 4) | 5;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] =  0;

            zoffsets[index][0] =  0;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::LEFT << 4) | 0;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] =  0;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 0;
        }
    }

    {   // Down
        {
            constexpr u32 index = ((u32) VoxelFaceDirection::DOWN << 4) | 1;
            xoffsets[index][0] = 0;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = 1;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 0;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::DOWN << 4) | 0;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] =  0;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = 0;
            zoffsets[index][1] = 1;
            zoffsets[index][2] = 1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::DOWN << 4) | 5;
            xoffsets[index][0] =  0;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] =  0;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::DOWN << 4) | 4;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 0;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] =  0;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }
    }

    {   // Back
        {
            constexpr u32 index = ((u32) VoxelFaceDirection::BACK << 4) | 6;
            xoffsets[index][0] =  0;
            xoffsets[index][1] = -1;
            xoffsets[index][2] = -1;

            yoffsets[index][0] = 1;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 0;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::BACK << 4) | 7;
            xoffsets[index][0] = 1;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 0;

            yoffsets[index][0] = 0;
            yoffsets[index][1] = 1;
            yoffsets[index][2] = 1;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::BACK << 4) | 4;
            xoffsets[index][0] = 0;
            xoffsets[index][1] = 1;
            xoffsets[index][2] = 1;

            yoffsets[index][0] = -1;
            yoffsets[index][1] = -1;
            yoffsets[index][2] =  0;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }

        {
            constexpr u32 index = ((u32) VoxelFaceDirection::BACK << 4) | 5;
            xoffsets[index][0] = -1;
            xoffsets[index][1] = -1;
            xoffsets[index][2] =  0;

            yoffsets[index][0] =  0;
            yoffsets[index][1] = -1;
            yoffsets[index][2] = -1;

            zoffsets[index][0] = -1;
            zoffsets[index][1] = -1;
            zoffsets[index][2] = -1;
        }
    }
}