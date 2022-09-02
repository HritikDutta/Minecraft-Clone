#pragma once

#include "engine/camera.h"
#include "engine/mesh.h"
#include "engine/transform.h"

namespace R3D
{

void Init();
void Shutdown();

void Begin(Camera& camera);
void End();

void RenderMesh(const Mesh& mesh, Transform& transform);

} // namespace R3D
