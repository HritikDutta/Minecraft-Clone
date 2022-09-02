#pragma once

#include "core/types.h"
#include "containers/darray.h"
#include "math/math.h"
#include "graphics/shader.h"
#include "engine/transform.h"

struct Submesh
{
    u32 vao, ibo;
    u32 indexCount;
    Transform transform;
};

struct Mesh
{
    DynamicArray<Submesh> submeshes;
    Shader* shader;
    void LoadFBX(DynamicArray<u8>& fileview);
};