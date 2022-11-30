#pragma once

#include "core/types.h"
#include "engine/camera.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "aabb.h"
#include "chunk_area.h"
#include "voxel.h"

namespace ChunkRenderer
{

void Init();
void Shutdown();

void Begin(Camera& camera, const Texture& texture);
void End();

struct DebugStats
{
    u32 trianglesRendered;
    u32 batches;
};

struct DebugSettings
{
    bool showWireframe = false;
    bool showBatches = false;
    bool showLighting = false;
};

void RenderChunkArea(VoxelChunkArea& area, Shader& shader, DebugStats& stats, const DebugSettings& settings, bool& updateTransparentBatch);

} // namespace ChunkRenderer