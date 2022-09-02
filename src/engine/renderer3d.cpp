#include "renderer3d.h"

#include "core/logging.h"
#include "engine/camera.h"
#include "engine/mesh.h"
#include "engine/transform.h"
#include "platform/platform.h"

#include <glad/glad.h>

namespace R3D
{

Camera* renderCamera = nullptr;

void Init()
{
}

void Shutdown()
{
}

void Begin(Camera& camera)
{
    renderCamera = &camera;
}

void End()
{
    renderCamera = nullptr;     // I can't think of anything else rn...
}

void RenderMesh(const Mesh& mesh, Transform& transform)
{
    AssertWithMessage(renderCamera != nullptr, "R3D::Begin() not called!");

    mesh.shader->Bind();
    for (auto& submesh : mesh.submeshes)
    {
        mesh.shader->SetUniformMatrix4("u_viewProjection", renderCamera->viewProjection());
        mesh.shader->SetUniformMatrix4("u_transform", transform.worldMatrix() * submesh.transform.worldMatrix());

        glBindVertexArray(submesh.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.ibo);

        glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, nullptr);
    }
}

} // namespace R3D
