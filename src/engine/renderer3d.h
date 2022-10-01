#pragma once

#include "engine/camera.h"
#include "engine/mesh.h"
#include "engine/skybox.h"
#include "engine/transform.h"
#include "graphics/shader.h"
#include "graphics/cubemap.h"

namespace R3D
{

void Init();
void Shutdown();

void Begin(Camera& camera);
void End();

void RenderMesh(const Mesh& mesh, Transform& transform);
void RenderSkybox(const Skybox& skybox);

} // namespace R3D
