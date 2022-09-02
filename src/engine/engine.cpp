#include "engine.h"

#include "core/application.h"
#include "game/chunk_renderer.h"
#include "renderer2D.h"
#include "renderer3d.h"
#include "imgui.h"

namespace Engine
{

void Init(const Application& app)
{
    // R2D::Init();
    // R3D::Init();
    
    Imgui::Init(app);
    ChunkRenderer::Init();
}

void Shutdown()
{
    Imgui::Shutdown();
    ChunkRenderer::Shutdown();

    // R3D::Shutdown();
    // R2D::Shutdown();
}

} // namespace Engine
