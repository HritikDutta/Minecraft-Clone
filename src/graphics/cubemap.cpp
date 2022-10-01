#include "cubemap.h"

#include "core/types.h"
#include "containers/stringview.h"
#include "containers/hashtable.h"

#include <stb_image.h>
#include <glad/glad.h>

static HashTable<String, Cubemap> loadedCubemaps;

// Pixels should be in this order:
//      right, left, top, bottom, front, back
static void InternalCubemapLoadPixels(Cubemap& cbm, u8* pixels[6], s32 width[6], s32 height[6], s32 bytesPP, const CubemapSettings& settings)
{
    GLint internalFormat, format;
    switch (bytesPP)
    {
        case 3:
        {
            internalFormat = GL_RGB8;
            format = GL_RGB;
        } break;

        case 4:
        {
            internalFormat = GL_RGBA8;
            format = GL_RGBA;
        } break;

        default:
        {
            AssertNotImplemented();
        } break;
    }

    glGenTextures(1, &cbm.cbmID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cbm.cbmID);

    {   // Send all 6 faces to the GPU
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, width[0], height[0], 0, format, GL_UNSIGNED_BYTE, pixels[0]);    // Right
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, width[1], height[1], 0, format, GL_UNSIGNED_BYTE, pixels[1]);    // Left
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, width[2], height[2], 0, format, GL_UNSIGNED_BYTE, pixels[2]);    // Top
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, width[3], height[3], 0, format, GL_UNSIGNED_BYTE, pixels[3]);    // Bottom
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, width[4], height[4], 0, format, GL_UNSIGNED_BYTE, pixels[4]);    // Front
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, width[5], height[5], 0, format, GL_UNSIGNED_BYTE, pixels[5]);    // Back
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, (GLint) settings.minFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, (GLint) settings.maxFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, (GLint) settings.wrapS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, (GLint) settings.wrapT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, (GLint) settings.wrapR);
}

// Give this a name so I can keep track of this
void Cubemap::LoadPixels(StringView name, u8* pixels[6], s32 width[6], s32 height[6], s32 bytesPP, const CubemapSettings& settings)
{
    auto cbm = loadedCubemaps.Find(name);
    if (cbm)
    {
        cbmID = (*cbm).value.cbmID;
        return;
    }

    InternalCubemapLoadPixels(*this, pixels, width, height, bytesPP, settings);

    loadedCubemaps[name] = *this;
}

void Cubemap::Load(StringView name, const StringView filepath[6], const CubemapSettings& settings)
{
    stbi_set_flip_vertically_on_load(false);
    
    auto tex = loadedCubemaps.Find(name);
    if (tex)
    {
        cbmID = (*tex).value.cbmID;
        return;
    }

    u8* pixels[6];
    s32 bytesPP, width[6], height[6];
    
    for (s32 index = 0; index < 6; index++)
    {
        pixels[index] = stbi_load(filepath[index].cstr(), width + index, height + index, &bytesPP, 0);
        AssertWithMessage(pixels, "Image couldn't be loaded!");
    }

    InternalCubemapLoadPixels(*this, pixels, width, height, bytesPP, settings);

    for (s32 i = 0; i < 6; i++)
        stbi_image_free(pixels[i]);

    loadedCubemaps[name] = *this;
}

void Cubemap::Free()
{
    if (cbmID)
        glDeleteTextures(1, &cbmID);
}

void Cubemap::Bind(int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cbmID);
}