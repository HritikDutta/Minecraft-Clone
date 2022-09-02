#pragma once

#include "core/types.h"
#include "containers/stringview.h"

#include <glad/glad.h>

struct TextureSettings
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

    Wrapping wrapS = Wrapping::REPEAT;
    Wrapping wrapT = Wrapping::REPEAT;

    static TextureSettings Default() { return TextureSettings(); }
};

struct Texture
{
    u32 texID;

    void Load(StringView filepath, const TextureSettings& settings);
    void LoadPixels(StringView name, u8* pixels, s32 width, s32 height, s32 bytesPP, const TextureSettings& settings);
    void Free();

    void Bind(int slot) const;

    static bool Exists(StringView name, Texture& outTexture);

    s32 width() const;
    s32 height() const;
};