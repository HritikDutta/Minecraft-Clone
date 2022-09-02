#include "renderer2d.h"

#include "core/logging.h"
#include "math/math.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "platform/platform.h"
#include "camera.h"
#include "shader_paths.h"
#include "batch.h"

#include <glad/glad.h>

namespace R2D
{

static constexpr s32 maxSpriteCount = 20000;
static constexpr s32 maxCircleCount = 20000;

static constexpr s32 maxTexCount = 10;
static s32 activeSlots[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

struct SpriteVertex
{
    Vector3 position;
    Vector2 texCoord;
    f32 texIndex;
};

struct CircleVertex
{
    Vector3 worldPosition;
    Vector3 localPosition;
    Vector4 color;
    Vector2 texCoord;
    f32 fade;
    f32 texIndex;
};

using R2DSpriteBatchData = BatchData<SpriteVertex, maxTexCount>;
using R2DCircleBatchData = BatchData<CircleVertex, maxTexCount>;

static struct
{
    Camera* currentCamera = nullptr;

    R2DSpriteBatchData spriteBatch;
    R2DCircleBatchData circleBatch;

    u32 vbo, ibo;

    Texture whiteTexture;

    void* batchSharedBuffer = nullptr;
} r2dData;

static void InitWhiteTexture(int width, int height)
{
    // Use existing white texture if available
    if (Texture::Exists("White Texture", r2dData.whiteTexture))
        return;

    u8* pixels = (u8*) PlatformAllocate(width * height * 4);
    PlatformSetMemory(pixels, 0xFF, width * height * 4);

    TextureSettings settings;
    settings.maxFilter = settings.minFilter = TextureSettings::Filter::NEAREST;
    r2dData.whiteTexture.LoadPixels("White Texture", pixels, width, height, 4, settings);

    PlatformFree(pixels);
}

static void InitBatches()
{
    constexpr size_t spriteBatchSize = 4 * maxSpriteCount;
    constexpr size_t circleBatchSize = 4 * maxCircleCount;
    r2dData.batchSharedBuffer = PlatformAllocate(spriteBatchSize * sizeof(SpriteVertex) + circleBatchSize * sizeof(CircleVertex));

    {   // Init Sprite Batch

        // Compile Shaders
        AssertWithMessage(
            r2dData.spriteBatch.shader.CompileFromFile(r2dSpriteVertShaderPath, Shader::Type::VERTEX_SHADER),
            "Failed to compile Sprite Vertex Shader"
        );

        AssertWithMessage(
            r2dData.spriteBatch.shader.CompileFromFile(r2dSpriteFragShaderPath, Shader::Type::FRAGMENT_SHADER),
            "Failed to compile Sprite Fragment Shader"
        );

        AssertWithMessage(
            r2dData.spriteBatch.shader.Link(),
            "Failed to link Sprite Shader"
        );

        r2dData.spriteBatch.elemVerticesBuffer = (SpriteVertex*) r2dData.batchSharedBuffer;

    }

    {   // Init Circle Batch

        // Compile Shaders
        AssertWithMessage(
            r2dData.circleBatch.shader.CompileFromFile(r2dCircleVertShaderPath, Shader::Type::VERTEX_SHADER),
            "Failed to compile Sprite Vertex Shader"
        );

        AssertWithMessage(
            r2dData.circleBatch.shader.CompileFromFile(r2dCircleFragShaderPath, Shader::Type::FRAGMENT_SHADER),
            "Failed to compile Sprite Fragment Shader"
        );

        AssertWithMessage(
            r2dData.circleBatch.shader.Link(),
            "Failed to link Sprite Shader"
        );

        r2dData.circleBatch.elemVerticesBuffer = (CircleVertex*) (r2dData.spriteBatch.elemVerticesBuffer + spriteBatchSize);

    }
}

void Init()
{
    // Setup OpenGL Buffers and Arrays

    constexpr u32 maxBufferSize = Max(sizeof(SpriteVertex) * maxSpriteCount, sizeof(CircleVertex) * maxCircleCount) * 4;

    glGenBuffers(1, &r2dData.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r2dData.vbo);
    glBufferData(GL_ARRAY_BUFFER, maxBufferSize, nullptr, GL_DYNAMIC_DRAW);

    {   // Sprite Vertex Attribute Layout
        glGenVertexArrays(1, &r2dData.spriteBatch.vao);
        glBindVertexArray(r2dData.spriteBatch.vao);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(SpriteVertex), (const void*) offsetof(SpriteVertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(SpriteVertex), (const void*) offsetof(SpriteVertex, texCoord));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 1, GL_FLOAT, false, sizeof(SpriteVertex), (const void*) offsetof(SpriteVertex, texIndex));
    }

    {   // Circle Vertex Attribute Layout
        glGenVertexArrays(1, &r2dData.circleBatch.vao);
        glBindVertexArray(r2dData.circleBatch.vao);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(CircleVertex), (const void*) offsetof(CircleVertex, worldPosition));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(CircleVertex), (const void*) offsetof(CircleVertex, localPosition));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(CircleVertex), (const void*) offsetof(CircleVertex, color));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, false, sizeof(CircleVertex), (const void*) offsetof(CircleVertex, texCoord));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 1, GL_FLOAT, false, sizeof(CircleVertex), (const void*) offsetof(CircleVertex, fade));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 1, GL_FLOAT, false, sizeof(CircleVertex), (const void*) offsetof(CircleVertex, texIndex));
    }
    
    // Set up Index Buffer

    constexpr u32 maxIndices = Max(maxSpriteCount, maxCircleCount) * 6;
    u32 indices[maxIndices];
    u32 offset = 0;
    for (int i = 0; i < maxIndices; i += 6)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    glGenBuffers(1, &r2dData.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r2dData.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    InitBatches();

    InitWhiteTexture(32, 32);
}

void Shutdown()
{
    PlatformFree(r2dData.batchSharedBuffer);
}

void Begin(Camera& camera)
{
    r2dData.currentCamera = &camera;

    {   // Begin Sprite Batch
        r2dData.spriteBatch.elemVerticesPtr = r2dData.spriteBatch.elemVerticesBuffer;
        r2dData.spriteBatch.nextActiveTexSlot = 0;
        r2dData.spriteBatch.elemCount = 0;
    }

    {   // Begin Circle Batch
        r2dData.circleBatch.elemVerticesPtr = r2dData.circleBatch.elemVerticesBuffer;
        r2dData.spriteBatch.nextActiveTexSlot = 0;
        r2dData.circleBatch.elemCount = 0;
    }
}

template <typename BatchType>
void FlushBatch(BatchType& batch)
{
    if (batch.elemCount == 0)
        return;
    
    Shader& shader = batch.shader;

    shader.Bind();

    // Set View Projection for the batch
    shader.SetUniformMatrix4("u_viewProjection", r2dData.currentCamera->viewProjection());    

    // Set all textures for the batch
    for (int i = 0; i < batch.nextActiveTexSlot; i++)
        batch.textures[i].Bind(i);

    shader.SetUniform1iv("u_textures", batch.nextActiveTexSlot, activeSlots);    

    glBindVertexArray(batch.vao);

    GLsizeiptr size = (u8*) batch.elemVerticesPtr - (u8*) batch.elemVerticesBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, r2dData.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, batch.elemVerticesBuffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r2dData.ibo);
    glDrawElements(GL_TRIANGLES, 6 * batch.elemCount, GL_UNSIGNED_INT, nullptr);
}

void End()
{
    AssertWithMessage(r2dData.currentCamera, "R2D::Begin() was never called!");

    FlushBatch(r2dData.spriteBatch);
    FlushBatch(r2dData.circleBatch);
}

void PushSprite(const Sprite& sprite, const Matrix4& transform)
{
    R2DSpriteBatchData& batch = r2dData.spriteBatch;

    if (batch.elemCount >= maxSpriteCount)
    {
        End();
        Begin(*r2dData.currentCamera);
    }

    // Find if texture has already been set to active
    int textureSlot = batch.nextActiveTexSlot;
    for (int i = 0; i < batch.nextActiveTexSlot; i++)
    {
        if (batch.textures[i].texID == sprite.atlas.texID)
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
            Begin(*r2dData.currentCamera);

            textureSlot = 0;
        }

        batch.textures[textureSlot] = sprite.atlas;
        batch.nextActiveTexSlot++;
    }

    f32 top  = 1.0f - sprite.pivot.y;
    f32 left = -sprite.pivot.x;
    f32 bottom = -sprite.pivot.y;
    f32 right = 1.0f - sprite.pivot.x;

    f32 z = 0.0f;

    batch.elemVerticesPtr->position = transform * Vector3(left, bottom, z);
    batch.elemVerticesPtr->texCoord = Vector2(sprite.texCoords.s, sprite.texCoords.v);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->position = transform * Vector3(right, bottom, z);
    batch.elemVerticesPtr->texCoord = Vector2(sprite.texCoords.u, sprite.texCoords.v);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;
    
    batch.elemVerticesPtr->position = transform * Vector3(right, top, z);
    batch.elemVerticesPtr->texCoord = Vector2(sprite.texCoords.u, sprite.texCoords.t);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->position = transform * Vector3(left, top, z);
    batch.elemVerticesPtr->texCoord = Vector2(sprite.texCoords.s, sprite.texCoords.t);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr++;

    batch.elemCount++;
}

void PushCircleTextured(const Matrix4& transform, const Texture& tex, const Vector4& tint)
{
    R2DCircleBatchData& batch = r2dData.circleBatch;

    if (batch.elemCount >= maxCircleCount)
    {
        End();
        Begin(*r2dData.currentCamera);
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
            Begin(*r2dData.currentCamera);

            textureSlot = 0;
        }

        batch.textures[textureSlot] = tex;
        batch.nextActiveTexSlot++;
    }

    constexpr f32 top  = 1.0f;
    constexpr f32 left = -1.0f;
    constexpr f32 bottom = -1.0f;
    constexpr f32 right = 1.0f;

    constexpr f32 z = 0.0f;

    const f32 fade = 0.005f;

    batch.elemVerticesPtr->worldPosition = transform * Vector3(left, bottom, z);
    batch.elemVerticesPtr->localPosition = Vector3(left, bottom, z);
    batch.elemVerticesPtr->color = tint;
    batch.elemVerticesPtr->texCoord = Vector2(0, 0);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr->fade = fade;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->worldPosition = transform * Vector3(right, bottom, z);
    batch.elemVerticesPtr->localPosition = Vector3(right, bottom, z);
    batch.elemVerticesPtr->color = tint;
    batch.elemVerticesPtr->texCoord = Vector2(1, 0);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr->fade = fade;
    batch.elemVerticesPtr++;
    
    batch.elemVerticesPtr->worldPosition = transform * Vector3(right, top, z);
    batch.elemVerticesPtr->localPosition = Vector3(right, top, z);
    batch.elemVerticesPtr->color = tint;
    batch.elemVerticesPtr->texCoord = Vector2(1, 1);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr->fade = fade;
    batch.elemVerticesPtr++;

    batch.elemVerticesPtr->worldPosition = transform * Vector3(left, top, z);
    batch.elemVerticesPtr->localPosition = Vector3(left, top, z);
    batch.elemVerticesPtr->color = tint;
    batch.elemVerticesPtr->texCoord = Vector2(0, 1);
    batch.elemVerticesPtr->texIndex = (f32) textureSlot;
    batch.elemVerticesPtr->fade = fade;
    batch.elemVerticesPtr++;

    batch.elemCount++;
}

void PushCircle(const Matrix4& transform, const Vector4& color)
{
    PushCircleTextured(transform, r2dData.whiteTexture, color);
}

} // namespace R2D
