#include "engine.h"

#include "core/application.h"
#include "game/chunk_renderer.h"
#include "renderer2D.h"
#include "renderer3d.h"
#include "imgui.h"
#include "skybox.h"

namespace Engine
{

void Init(const Application& app)
{
    // R2D::Init();
    
    Imgui::Init(app);
    R3D::Init();
    ChunkRenderer::Init();
    Skybox::Init();
}

void Shutdown()
{
    Skybox::Shutdown();
    Imgui::Shutdown();
    R3D::Shutdown();
    ChunkRenderer::Shutdown();

    // R2D::Shutdown();
}

} // namespace Engine
