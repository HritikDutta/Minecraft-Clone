#include "skybox.h"

#include "core/types.h"
#include "graphics/cubemap.h"
#include "graphics/shader.h"
#include "engine/shader_paths.h"

struct
{
    u32 vao, vbo;
    Shader shader;
} skyboxData;

void Skybox::Init()
{
    f32 skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxData.vao);
    glBindVertexArray(skyboxData.vao);

    glGenBuffers(1, &skyboxData.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxData.vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), nullptr);

    {   // Compile Shader
        AssertWithMessage(
            skyboxData.shader.CompileFromFile(skyboxVertShaderPath, Shader::Type::VERTEX_SHADER),
            "Failed to compile Skybox Vertex Shader"
        );

        AssertWithMessage(
            skyboxData.shader.CompileFromFile(skyboxFragShaderPath, Shader::Type::FRAGMENT_SHADER),
            "Failed to compile Skybox Fragment Shader"
        );

        AssertWithMessage(
            skyboxData.shader.Link(),
            "Failed to link Skybox Shader"
        );
    }
}

void Skybox::Shutdown()
{
    glDeleteBuffers(1, &skyboxData.vbo);
    glDeleteVertexArrays(1, &skyboxData.vao);
}

Shader& Skybox::shader()
{
    return skyboxData.shader;
}

u32 Skybox::vao()
{
    return skyboxData.vao;
}

u32 Skybox::vbo()
{
    return skyboxData.vbo;
}

constexpr u32 Skybox::vertexArraySize()
{
    return 36;
}