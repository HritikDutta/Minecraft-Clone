#include "texture.h"

#include "core/types.h"
#include "containers/stringview.h"
#include "containers/hashtable.h"

#include <stb_image.h>
#include <glad/glad.h>

static HashTable<String, Texture> loadedTextures;

// OpenGL generates textureIDs sequentially so
// this way extra data about the texture can be accessed
// by using them as an index into an array rather than
// copying them everywhere.

struct TextureData
{
    s32 width, height;
    String name;
};

constexpr u32 maxLoadedTextures = 100;
static TextureData textureDataTable[maxLoadedTextures] = {};

static void InternalTextureLoadPixels(StringView name, Texture& tex, u8* pixels, s32 width, s32 height, s32 bytesPP, const TextureSettings& settings)
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

    glGenTextures(1, &tex.texID);
    glBindTexture(GL_TEXTURE_2D, tex.texID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) settings.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint) settings.maxFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint) settings.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint) settings.wrapT);

    textureDataTable[tex.texID].width = width;
    textureDataTable[tex.texID].height = height;
    textureDataTable[tex.texID].name = name;
}

void Texture::Load(StringView filepath, const TextureSettings& settings)
{
    stbi_set_flip_vertically_on_load(true);
    
    auto tex = loadedTextures.Find(filepath);
    if (tex)
    {
        texID = (*tex).value.texID;
        return;
    }
    
    s32 bytesPP, width, height;
    u8* pixels = stbi_load(filepath.cstr(), &width, &height, &bytesPP, 0);
    AssertWithMessage(pixels, "Image couldn't be loaded!");

    InternalTextureLoadPixels(filepath, *this, pixels, width, height, bytesPP, settings);

    stbi_image_free(pixels);

    loadedTextures[filepath] = *this;
}

// Give this a name so I can keep track of this
void Texture::LoadPixels(StringView name, u8* pixels, s32 width, s32 height, s32 bytesPP, const TextureSettings& settings)
{
    auto tex = loadedTextures.Find(name);
    if (tex)
    {
        texID = (*tex).value.texID;
        return;
    }

    InternalTextureLoadPixels(name, *this, pixels, width, height, bytesPP, settings);

    loadedTextures[name] = *this;
}

void Texture::Free()
{
    if (texID)
    {
        loadedTextures.Remove(textureDataTable[texID].name);
        glDeleteTextures(1, &texID);
    }
}

// Assuming there are 32 texture slots in the GPU
static u32 boundTextures[32] = {};

void Texture::Bind(int slot) const
{
    // Only bind if the texture wasn't bound before
    if (boundTextures[slot] != texID)
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texID);
        
        boundTextures[slot] = texID;
    }
}

bool Texture::Exists(StringView name, Texture& outTexture)
{
    auto tex = loadedTextures.Find(name);
    if (tex)
    {
        outTexture = (*tex).value;
        return true;
    }

    return false;
}

s32 Texture::width() const
{
    return textureDataTable[texID].width;
}

s32 Texture::height() const
{
    return textureDataTable[texID].height;
}