#include "imgui.h"

#include "core/types.h"
#include "core/logging.h"
#include "core/application.h"
#include "core/input.h"
#include "math/math.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "fileio/fileio.h"
#include "shader_paths.h"
#include "serialization/json.h"
#include "containers/stringview.h"
#include "containers/hashtable.h"
#include "batch.h"

#include <glad/glad.h>

#define ImguiInvalidID (ID { -1, -1 })

namespace Imgui
{

static const Application* activeApp = nullptr;

static constexpr s32 maxQuadCount = 2000;
static constexpr s32 maxTexCount = 10;
static s32 activeSlots[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

struct Vertex
{
    Vector3 position;
    Vector2 texCoord;
    Vector4 color;
    float texIndex;
};

using ImguiBatchData = BatchData<Vertex, maxTexCount>;

static struct
{
    u32 vbo, ibo;

    ImguiBatchData quadBatch;
    ImguiBatchData fontBatch;

    Texture whiteTexture;

    ID hot, active;

    Vertex* batchSharedBuffer = nullptr;
} uidata;

static void InitWhiteTexture(int width, int height)
{
    // Use existing white texture if available
    if (Texture::Exists("White Texture", uidata.whiteTexture))
        return;
    
    u8* pixels = (u8*) PlatformAllocate(width * height * 4);
    PlatformSetMemory(pixels, 0xFF, width * height * 4);

    TextureSettings settings;
    settings.maxFilter = settings.minFilter = TextureSettings::Filter::NEAREST;
    uidata.whiteTexture.LoadPixels("White Texture", pixels, width, height, 4, settings);

    PlatformFree(pixels);
}

static void InitBatches()
{
    // Set up batching for elems
    constexpr size_t batchSize = 4 * maxQuadCount;
    uidata.batchSharedBuffer = (Vertex*) PlatformAllocate(2 * batchSize * sizeof(Vertex));

    {   // Init Quad Batch
        
        // Compile Shaders
        AssertWithMessage(
            uidata.quadBatch.shader.CompileFromFile(uiQuadVertShaderPath, Shader::Type::VERTEX_SHADER),
            "Failed to compile UI Quad Vertex Shader!"
        );
        
        AssertWithMessage(
            uidata.quadBatch.shader.CompileFromFile(uiQuadFragShaderPath, Shader::Type::FRAGMENT_SHADER),
            "Failed to compile UI Quad Fragment Shader!"
        );

        AssertWithMessage(
            uidata.quadBatch.shader.Link(),
            "Failed to link UI Quad Shader!"
        );

        uidata.quadBatch.elemVerticesBuffer = uidata.batchSharedBuffer;

    }

    {   // Init Font Batch
    
        // Compile Shaders
        AssertWithMessage(
            uidata.fontBatch.shader.CompileFromFile(uiFontVertShaderPath, Shader::Type::VERTEX_SHADER),
            "Failed to compile UI Font Vertex Shader!"
        );
        
        AssertWithMessage(
            uidata.fontBatch.shader.CompileFromFile(uiFontFragShaderPath, Shader::Type::FRAGMENT_SHADER),
            "Failed to compile UI Font Fragment Shader!"
        );

        AssertWithMessage(
            uidata.fontBatch.shader.Link(),
            "Failed to link UI Font Shader!"
        );

        uidata.fontBatch.elemVerticesBuffer = uidata.quadBatch.elemVerticesBuffer + batchSize;

    }
}

void Init(const Application& app)
{
    // Set current platform state
    AssertWithMessage(activeApp == nullptr, "Imgui was already initialized!");
    activeApp = &app;

    u32 vao;

    glGenBuffers(1, &uidata.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, uidata.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * maxQuadCount * 4, nullptr, GL_DYNAMIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(Vertex), (const void*) offsetof(Vertex, texIndex));

    uidata.fontBatch.vao = uidata.quadBatch.vao = vao;
    
    u32 indices[maxQuadCount * 6];
    u32 offset = 0;
    for (int i = 0; i < maxQuadCount * 6; i += 6)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    glGenBuffers(1, &uidata.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uidata.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    uidata.hot = uidata.active = ImguiInvalidID;

    InitBatches();

    InitWhiteTexture(2, 2);
}

void Shutdown()
{
    AssertWithMessage(activeApp, "Imgui was never initialized!");

    uidata.whiteTexture.Free();
    PlatformFree(uidata.batchSharedBuffer);

    // No need to reset other things since this will be called at the end of the program
}

void Begin()
{
    AssertWithMessage(activeApp, "Imgui was never initialized!");

    {   // Begin Quad Batch
        uidata.quadBatch.elemVerticesPtr = uidata.quadBatch.elemVerticesBuffer;
        uidata.quadBatch.nextActiveTexSlot = 0;
        uidata.quadBatch.elemCount = 0;
    }

    {   // Begin Font Batch
        uidata.fontBatch.elemVerticesPtr = uidata.fontBatch.elemVerticesBuffer;
        uidata.fontBatch.nextActiveTexSlot = 0;
        uidata.fontBatch.elemCount = 0;
    }
}

static void FlushBatch(ImguiBatchData& batch)
{
    if (batch.elemCount == 0)
        return;
    
    batch.shader.Bind();

    // Set all textures for the batch
    for (int i = 0; i < batch.nextActiveTexSlot; i++)
        batch.textures[i].Bind(i);

    batch.shader.SetUniform1iv("u_textures", batch.nextActiveTexSlot, activeSlots);

    glBindVertexArray(batch.vao);

    // Bind and Update Data
    GLsizeiptr size = (u8*) batch.elemVerticesPtr - (u8*) batch.elemVerticesBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, uidata.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, batch.elemVerticesBuffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uidata.ibo);
    glDrawElements(GL_TRIANGLES, 6 * batch.elemCount, GL_UNSIGNED_INT, nullptr);
}

void End()
{
    AssertWithMessage(activeApp, "Imgui was never initialized!");

    glDisable(GL_DEPTH_TEST);

    FlushBatch(uidata.quadBatch);
    FlushBatch(uidata.fontBatch);

    glEnable(GL_DEPTH_TEST);
}

static inline s32 GetKerningIndex(s32 a, s32 b)
{
    // Since all unicodes are less than 128
    return ((a << 8) | b);
}

void Font::Load(StringView atlaspath, StringView datapath)
{
    {   // Load font atlas
        texture.Load(atlaspath, TextureSettings::Default());
    }

    {   // Load font data
        String json;
        LoadFileToString(datapath, json);

        json::Document document;
        json::ParseJsonString(json.cstr(), document);

        json::Value data = document.Start();

        size = data["atlas"]["size"].int64();

        const json::Value& metrics = data["metrics"];
        lineHeight = metrics["lineHeight"].float64();
        ascender = metrics["ascender"].float64();
        descender = metrics["descender"].float64();

        for (const auto& glyph : data["glyphs"].array())
        {
            u32 unicode = glyph["unicode"].int64();

            GlyphData& glyphData = glyphs[unicode - ' '];
            glyphData.advance = glyph["advance"].float64();

            {   // Plane bounds
                const json::Value& planeBounds = glyph["planeBounds"];
                if (!planeBounds.IsNull())
                {
                    glyphData.planeBounds = Vector4(
                        planeBounds["left"].float64(),
                        planeBounds["bottom"].float64(),
                        planeBounds["right"].float64(),
                        planeBounds["top"].float64()
                    );
                }
            }
            
            {   // Atlas bounds
                const json::Value& atlasBounds = glyph["atlasBounds"];
                if (!atlasBounds.IsNull())
                {
                    glyphData.atlasBounds = Vector4(
                        atlasBounds["left"].float64()   / texture.width(),
                        atlasBounds["top"].float64()    / texture.height(),
                        atlasBounds["right"].float64()  / texture.width(),
                        atlasBounds["bottom"].float64() / texture.height()
                    );
                }
            }
        }

        for (const auto& kerning : data["kerning"].array())
        {
            s32 kIndex = GetKerningIndex(kerning["unicode1"].int64(), kerning["unicode2"].int64());
            kerningTable[kIndex] = kerning["advance"].float64();
        }
    }
}
void Font::Free()
{
    texture.Free();
}

Vector2 GetRenderedTextSize(StringView text, Font& font, f32 size)
{
    size = (size < 0.0f) ? font.size : size;

    Vector2 position;
    position.y += size * (font.ascender);

    Vector2 totalSize = position;

    int lineStart = 0;
    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            totalSize.y += size * font.lineHeight;
            position.y += size * font.lineHeight;

            totalSize.x = Max(totalSize.x, position.x);
            position.x = 0.0f;
            
            lineStart = i + 1;
            continue;
        }

        if (text[i] == '\r')
        {
            position.x = 0.0f;
            continue;
        }

        if (text[i] == '\t')
        {
            f32 x = size * font.glyphs[0].advance;
            position.x += x * (4 - ((i - lineStart) % 4));
            continue;
        }

        if (i > 0)
        {
            s32 kerningIndex = GetKerningIndex(text[i], text[i - 1]);
            if (font.kerningTable.Find(kerningIndex))
                position.x += size * font.kerningTable[kerningIndex];
        }

        Font::GlyphData& glyph = font.glyphs[text[i] - ' '];
        position.x += size * glyph.advance;
    }

    totalSize.x = Max(totalSize.x, position.x);

    return totalSize;
}


Vector2 GetRenderedCharSize(char ch, Font& font, f32 size)
{
    if (ch == ' '  ||
        ch == '\n' ||
        ch == '\r' ||
        ch == '\t')
        return Vector2();

    size = (size < 0.0f) ? font.size : size;

    Font::GlyphData& glyph = font.glyphs[ch - ' '];
    return Vector2(size * glyph.advance, size * font.lineHeight);
}

static void PushUIQuad(ImguiBatchData& batch, const Rect& rect, const Vector4& texCoords, const Texture& tex, const Vector4& color)
{
    AssertWithMessage(activeApp, "Imgui was never initialized!");

    if (batch.elemCount >= maxQuadCount)
    {
        End();
        Begin();
    }

    // Find if texture has already been set to active
    int textureSlot = batch.nextActiveTexSlot;
    for (int i = 0; i < batch.nextActiveTexSlot; i++)
    {
        if (batch.textures[i].texID == tex.texID)
        {
            textureSlot = i;
            break;
        }
    }

    if (textureSlot == batch.nextActiveTexSlot)
    {
        // End the batch if all the texture slots are occupied
        if (textureSlot >= maxTexCount)
        {
            End();
            Begin();

            textureSlot = 0;
        }

        batch.textures[textureSlot] = tex;
        batch.nextActiveTexSlot++;
    }

    f32 top    = 1.0f - 2.0f * (rect.topLeft.y / activeApp->window.refHeight);
    f32 left   = 2.0f * (rect.topLeft.x / activeApp->window.refWidth) - 1.0f;
    f32 right  = 2.0f * ((rect.topLeft.x + rect.size.x) / activeApp->window.refWidth) - 1.0f;
    f32 bottom = 1.0f - 2.0f * ((rect.topLeft.y + rect.size.y) / activeApp->window.refHeight);

    f32 z = rect.topLeft.z;

    batch.elemVerticesPtr->position = Vector3(left, bottom, z);
    batch.elemVerticesPtr->texCoord = Vector2(texCoords.s, texCoords.v);
    batch.elemVerticesPtr->color = color;
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->position = Vector3(right, bottom, z);
    batch.elemVerticesPtr->texCoord = Vector2(texCoords.u, texCoords.v);
    batch.elemVerticesPtr->color = color;
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->position = Vector3(right, top, z);
    batch.elemVerticesPtr->texCoord = Vector2(texCoords.u, texCoords.t);
    batch.elemVerticesPtr->color = color;
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->position = Vector3(left, top, z);
    batch.elemVerticesPtr->texCoord = Vector2(texCoords.s, texCoords.t);
    batch.elemVerticesPtr->color = color;
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemCount++;
}

void RenderRect(const Rect& rect, const Vector4& color)
{
    Vector4 texCoords { 0.0f, 0.0f, 1.0f, 1.0f };
    PushUIQuad(uidata.quadBatch, rect, texCoords, uidata.whiteTexture, color);
}

void RenderImage(const Image& image, const Vector3& topLeft, const Vector2& scale, const Vector4& tint)
{
    Vector4 texCoords { 0.0f, 1.0f, 1.0f, 0.0f };

    Rect rect;
    rect.topLeft = topLeft;
    rect.size = Vector2(image.width(), image.height()) * scale;

    PushUIQuad(uidata.quadBatch, rect, texCoords, image, tint);
}

bool RenderButton(ID id, const Rect& rect, const Vector4& defaultColor, const Vector4& hoverColor, const Vector4& pressedColor)
{
    bool clicked = false;

    Vector4 color = defaultColor;
    Vector2 mpos = Input::MousePosition();

    if (mpos.x >= rect.topLeft.x && mpos.x <= rect.topLeft.x + rect.size.x &&
        mpos.y >= rect.topLeft.y && mpos.y <= rect.topLeft.y + rect.size.y)
    {
        uidata.hot = id;

        if (Input::GetMouseButtonDown(MouseButton::LEFT))
        {
            clicked = uidata.active != id;
            uidata.active = id;
        }
        
        if (uidata.active == id && Input::GetMouseButtonUp(MouseButton::LEFT))
            uidata.active = ImguiInvalidID;

        color = (uidata.active == id) ? pressedColor : hoverColor;
    }
    else
    {
        if (uidata.hot == id)
            uidata.hot = ImguiInvalidID;
    }

    RenderRect(rect, color);

    return clicked;
}

void RenderText(StringView text, Font& font, const Vector3& topLeft, f32 size, const Vector4& tint)
{
    size = (size < 0) ? font.size : size;

    Vector3 position = topLeft;
    position.y += size * (font.ascender);

    int lineStart = 0;
    for (int i = 0; i < text.length(); i++)
    {
        if (text[i] == '\n')
        {
            position.y += size * font.lineHeight;
            position.x = topLeft.x;
            lineStart = i + 1;
            continue;
        }

        if (text[i] == '\r')
        {
            position.x = topLeft.x;
            continue;
        }

        if (text[i] == '\t')
        {
            f32 x = size * font.glyphs[0].advance;
            position.x += x * (4 - ((i - lineStart) % 4));
            continue;
        }

        Font::GlyphData& glyph = font.glyphs[text[i] - ' '];

        Rect rect;
        rect.topLeft = position + Vector3(size * glyph.planeBounds.s, size * -glyph.planeBounds.v, 0.0f);
        rect.size = size * Vector2(glyph.planeBounds.u - glyph.planeBounds.s, glyph.planeBounds.v - glyph.planeBounds.t);

        if (i > 0)
        {
            s32 kerningIndex = GetKerningIndex(text[i], text[i-1]);
            if (font.kerningTable.Find(kerningIndex))
            {
                rect.topLeft.x += size * font.kerningTable[kerningIndex];
                position.x += size * font.kerningTable[kerningIndex];
            }
        }

        PushUIQuad(uidata.fontBatch, rect, glyph.atlasBounds, font.texture, tint);

        position.x += size * glyph.advance;
    }
}


void RenderChar(char ch, Font& font, const Vector3& topLeft, f32 size, const Vector4& tint)
{
    if (ch == ' '  ||
        ch == '\n' ||
        ch == '\r' ||
        ch == '\t')
        return;

    size = (size < 0) ? font.size : size;

    Vector3 position = topLeft;
    position.y += size * (font.ascender);

    Font::GlyphData& glyph = font.glyphs[ch - ' '];

    Rect rect;
    rect.topLeft = position + Vector3(size * glyph.planeBounds.s, size * -glyph.planeBounds.v, 0.0f);
    rect.size = size * Vector2(glyph.planeBounds.u - glyph.planeBounds.s, glyph.planeBounds.v - glyph.planeBounds.t);

    PushUIQuad(uidata.fontBatch, rect, glyph.atlasBounds, font.texture, tint);
}

} // namespace Imgui