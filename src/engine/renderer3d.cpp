#include "renderer3d.h"

#include "core/logging.h"
#include "engine/camera.h"
#include "engine/mesh.h"
#include "engine/transform.h"
#include "platform/platform.h"

#include <glad/glad.h>

namespace R3D
{

struct
{
    Camera* renderCamera = nullptr;
} r3dData;

void Init()
{
}

void Shutdown()
{
}

void Begin(Camera& camera)
{
    r3dData.renderCamera = &camera;
}

void End()
{
    r3dData.renderCamera = nullptr;     // I can't think of anything else rn...
}

void RenderMesh(const Mesh& mesh, Transform& transform)
{
    AssertWithMessage(r3dData.renderCamera != nullptr, "R3D::Begin() not called!");

    mesh.shader->Bind();
    for (auto& submesh : mesh.submeshes)
    {
        mesh.shader->SetUniformMatrix4("u_viewProjection", r3dData.renderCamera->viewProjection());
        mesh.shader->SetUniformMatrix4("u_transform", transform.worldMatrix() * submesh.transform.worldMatrix());

        glBindVertexArray(submesh.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, submesh.ibo);

        glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, nullptr);
    }
}

void RenderSkybox(const Skybox& skybox)
{
    AssertWithMessage(r3dData.renderCamera != nullptr, "R3D::Begin() not called!");

    Shader& shader = Skybox::shader();

    shader.Bind();

    skybox.cubemap.Bind(0);
    shader.SetUniform1i("u_skybox", 0);

    {
        Matrix4 viewWithoutTranslation = r3dData.renderCamera->view();

        viewWithoutTranslation.data[3][0] = 0.0f;
        viewWithoutTranslation.data[3][1] = 0.0f;
        viewWithoutTranslation.data[3][2] = 0.0f;
        viewWithoutTranslation.data[0][3] = 0.0f;
        viewWithoutTranslation.data[1][3] = 0.0f;
        viewWithoutTranslation.data[2][3] = 0.0f;
        viewWithoutTranslation.data[3][3] = 1.0f;

        shader.SetUniformMatrix4("u_matrix", r3dData.renderCamera->projection() * viewWithoutTranslation);
    }

    glBindVertexArray(Skybox::vao());
    glBindBuffer(GL_ARRAY_BUFFER, Skybox::vbo());

    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

} // namespace R3D
