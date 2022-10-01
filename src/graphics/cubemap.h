#pragma once

#include "core/types.h"
#include "containers/stringview.h"

#include <glad/glad.h>

struct CubemapSettings
{
    enum struct Filter
    {
        NEAREST = GL_NEAREST,
        LINEAR  = GL_LINEAR
    };

    enum struct Wrapping
    {
        CLAMP           = GL_CLAMP,
        CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
        CLAMP_TO_EDGE   = GL_CLAMP_TO_EDGE,
        MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
        REPEAT          = GL_REPEAT
    };

    Filter minFilter = Filter::LINEAR;
    Filter maxFilter = Filter::LINEAR;

    Wrapping wrapS = Wrapping::CLAMP_TO_EDGE;
    Wrapping wrapT = Wrapping::CLAMP_TO_EDGE;
    Wrapping wrapR = Wrapping::CLAMP_TO_EDGE;

    static CubemapSettings Default() { return CubemapSettings(); }
};

// Faces should be in this order:
//      right, left, top, bottom, front, back
struct Cubemap
{
    u32 cbmID;

    void Load(StringView name, const StringView filepath[6], const CubemapSettings& settings);
    void LoadPixels(StringView name, u8* pixels[6], s32 width[6], s32 height[6], s32 bytesPP, const CubemapSettings& settings);
    void Free();

    void Bind(int slot) const;

    static bool Exists(StringView name, Cubemap& outTexture);

    s32 width() const;
    s32 height() const;
};