#include "core/application.h"
#include "core/input.h"
#include "engine/imgui.h"
#include "engine/camera.h"
#include "engine/renderer3d.h"
#include "engine/shader_paths.h"
#include "game/chunk_area.h"
#include "game/chunk_renderer.h"
#include "game/voxel_physics.h"
#include "game/voxel.h"
#include "graphics/texture.h"
#include "math/math.h"

#include <SimplexNoise.h>

struct SceneData
{
    // Data
    VoxelChunkArea area;

    // Gameplay
    BlockType currentBlockType = BlockType::DIRT;

    // Rendering
    Shader  voxelShader;
    Texture voxelTextureAtlas;
    Texture whiteTexture;       // For debugging lighting
    Texture currentTexture;

    // World Generation
    SimplexNoise noise;

    // Camera
    Camera camera;
    f32 cameraMoveSpeed = 10.5f;
    f32 cameraLookSpeed = 0.5f;

    // UI
    Imgui::Font  font;
    Imgui::Image crosshair;

    // Debugging
    DebugRendererStats    debugStats;
    DebugRendererSettings debugSettings;

    bool showStats = false;
    bool freeLook  = true;
};

void OnInit(Application& app)
{
    SceneData& scene = *(SceneData*) app.data;

    {   // Camera
        const f32 aspectRatio = (f32) app.window.width / (f32) app.window.height;
        scene.camera = Camera::Perspective(45, aspectRatio, 0.1f, 1000.0f);
        scene.camera.position() = Vector3(-4, 3, 3);
        scene.camera.forward() = (-scene.camera.position()).Normalized();
        scene.camera.up() = Vector3::up;
        scene.camera.UpdateDirections();
        scene.camera.UpdateYawAndPitch();
        scene.camera.UpdateViewMatrix();
    }

    {   // Compile Shader
        AssertWithMessage(
            scene.voxelShader.CompileFromFile(voxelVertShaderPath, Shader::Type::VERTEX_SHADER),
            "Failed to compile Voxel Vertex Shader"
        );

        AssertWithMessage(
            scene.voxelShader.CompileFromFile(voxelFragShaderPath, Shader::Type::FRAGMENT_SHADER),
            "Failed to compile Voxel Fragment Shader"
        );

        AssertWithMessage(
            scene.voxelShader.Link(),
            "Failed to link Voxel Shader"
        );
    }

    {   // Init voxel chunk area data
        scene.area.Create(120.0f);
        scene.area.InitializeChunkArea(scene.noise, scene.camera.position());
    }

    {   // Load Texture Atlas
        TextureSettings settings = TextureSettings::Default();
        settings.minFilter = settings.maxFilter = TextureSettings::Filter::NEAREST;
        scene.voxelTextureAtlas.Load("assets/art/atlas/Minecraft Atlas.png", settings);

        if (!Texture::Exists("White Texture", scene.whiteTexture))
        {
            constexpr u32 dimension = 2;
            u8 pixels[dimension * dimension];
            PlatformSetMemory(pixels, 0xFF, dimension * dimension * 4);

            TextureSettings settings = TextureSettings::Default();
            settings.minFilter = settings.maxFilter = TextureSettings::Filter::NEAREST;
            scene.whiteTexture.LoadPixels("White Texture", pixels, dimension, dimension, 4, settings);
        }

        scene.currentTexture = (scene.debugSettings.showLighting) ? scene.whiteTexture : scene.voxelTextureAtlas;
    }

    {   // UI Stuff
    
        // Font
        scene.font.Load("assets/fonts/bell.font.png", "assets/fonts/bell.font.json");

        {   // Crosshair
            TextureSettings settings = TextureSettings::Default();
            settings.minFilter = settings.maxFilter = TextureSettings::Filter::NEAREST;
            scene.crosshair.Load("assets/art/ui/crosshair.png", settings);
        }

    }

    Input::CenterMouse(scene.freeLook);

    app.ShowCursor(!scene.freeLook);

    String::ResetPool();    // No need to do this...
}

void OnUpdate(Application& app)
{
    if (Input::GetKeyDown(Key::ESCAPE))
    {
        app.Exit();
        return;
    }

    SceneData& scene = *(SceneData*) app.data;

    #ifdef GN_DEBUG

    if (Input::GetKeyDown(Key::GRAVE))
    {
        if (Input::GetKey(Key::CONTROL))
        {
            scene.freeLook = !scene.freeLook;
            Input::CenterMouse(scene.freeLook);
            app.ShowCursor(!scene.freeLook);
        }
        else
            scene.showStats = !scene.showStats;
    }
    
    if (Input::GetKey(Key::CONTROL))
    {
        if (Input::GetKeyDown(Key::W))
            scene.debugSettings.showWireframe = !scene.debugSettings.showWireframe;
            
        if (Input::GetKeyDown(Key::B))
            scene.debugSettings.showBatches = !scene.debugSettings.showBatches;
            
        if (Input::GetKeyDown(Key::L))
        {
            scene.debugSettings.showLighting = !scene.debugSettings.showLighting;
            scene.currentTexture = (scene.debugSettings.showLighting) ? scene.whiteTexture : scene.voxelTextureAtlas;
        }
    }

    #endif // GN_DEBUG

    // TODO: Use scroll input instead
    if (Input::GetKeyDown(Key::LEFT))
        scene.currentBlockType = (BlockType) Wrap((s32) scene.currentBlockType - 1, 1, (s32) BlockType::NUM_TYPES);
        
    if (Input::GetKeyDown(Key::RIGHT))
        scene.currentBlockType = (BlockType) Wrap((s32) scene.currentBlockType + 1, 1, (s32) BlockType::NUM_TYPES);

    MoveCamera(scene.camera, scene.cameraLookSpeed, scene.cameraMoveSpeed, app.deltaTime, scene.freeLook);
    
    scene.area.UpdateChunkArea(scene.noise, scene.camera.position());

    // Remove blocks
    if (Input::GetMouseButtonDown(MouseButton::LEFT))
    {
        RayHitResult hit;
        if (RayIntersectionWithBlock(scene.area, scene.camera.position(), scene.camera.forward(), hit, 4.0f))
            PlaceBlockAtPosition(scene.area, hit.chunkIndex, hit.blockIndex, BlockType::NONE);
    }

    // Place Blocks
    if (Input::GetMouseButtonDown(MouseButton::RIGHT))
    {
        RayHitResult hit;
        if (RayIntersectionWithBlock(scene.area, scene.camera.position(), scene.camera.forward(), hit, 4.0f))
        {
            Vector3Int blockIndex = hit.blockIndex + hit.normal;
            Vector3Int chunkIndex = hit.chunkIndex;

            CorrectBlockIndex(chunkIndex, blockIndex);
            PlaceBlockAtPosition(scene.area, chunkIndex, blockIndex, scene.currentBlockType);
        }
    }
}

void OnRender(Application& app)
{
    SceneData& scene = *(SceneData*) app.data;
    
    ChunkRenderer::Begin(scene.camera, scene.currentTexture);

    ChunkRenderer::RenderChunkArea(scene.area, scene.voxelShader, scene.debugStats, scene.debugSettings);

    ChunkRenderer::End();

    Imgui::Begin();

    {   // Render crosshair
        Vector3 topLeft = Vector3((app.window.refWidth - scene.crosshair.width()) / 2.0f, (app.window.refHeight - scene.crosshair.height()) / 2.0f, 0.0f);
        Imgui::RenderImage(scene.crosshair, topLeft);
    }

    {   // Render selected block type name
        constexpr f32 fontSize = 24.0f;

        StringView name = blockTypeNames[(u32) scene.currentBlockType];
        Vector2 size = Imgui::GetRenderedTextSize(name, scene.font, fontSize);
        Vector3 topLeft = Vector3((app.window.refWidth - size.x - 10.0f) / 2.0f, app.window.refHeight - size.y - 10.0f, 0.0f);
        Imgui::RenderText(name, scene.font, topLeft, fontSize);
    }

    #ifdef GN_DEBUG

    // Render Stats
    if (scene.showStats)
    {
        #define KB (1024)
        #define MB (1024 * KB)
        #define GB (1024 * MB)

        f32 mem = (f32) PlatformGetMemoryAllocated() / (f32) GB;

        char buffer[128];
        sprintf(buffer, "FPS: %.2f\nTris: %u\nBatches: %u\nMem: %.2f GB", 1.0f / app.deltaTime, scene.debugStats.trianglesRendered, scene.debugStats.batches, mem);
        Imgui::RenderText(buffer, scene.font, Vector3(20, 10, 0), 24);
    }

    #endif // GN_DEBUG

    Imgui::End();
}

void OnWindowResize(Application& app)
{
    SceneData& scene = *(SceneData*) app.data;
    const f32 aspectRatio = (f32) app.window.width / (f32) app.window.height;
    scene.camera.SetProjection(Matrix4::Perspective(45, aspectRatio, 0.1f, 1000.0f));
}

void OnShutdown(Application& app)
{
    SceneData& scene = *(SceneData*) app.data;

    scene.area.Free();
    
    PlatformFree(app.data);     // Not really necessary
}

void CreateApp(Application& app)
{
    app.window.x = 200;
    app.window.y = 200;
    app.window.width = 1024;
    app.window.height = 720;
    app.window.name = "Minecraft Clone";

    app.data = PlatformAllocate(sizeof(SceneData));
    *(SceneData*) app.data = SceneData();

    app.OnInit = OnInit;
    app.OnUpdate = OnUpdate;
    app.OnRender = OnRender;
    app.OnShutdown = OnShutdown;
    app.OnWindowResize = OnWindowResize;

    app.clearColor = Vector4(0, 0.4, 1, 1);
}