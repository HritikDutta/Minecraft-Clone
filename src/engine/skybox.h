#pragma once

#include "graphics/cubemap.h"
#include "graphics/shader.h"

struct Skybox
{
    Cubemap cubemap;

    static void Init();
    static void Shutdown();

    static Shader& shader();
    static u32 vao();
    static u32 vbo();
    static constexpr u32 vertexArraySize();
};