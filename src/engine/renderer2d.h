#pragma once

#include "math/math.h"
#include "graphics/texture.h"
#include "camera.h"

namespace R2D
{

void Init();
void Shutdown();

void Begin(Camera& camera);
void End();

struct Sprite
{
    Texture atlas;
    Vector4 texCoords;
    Vector2 pivot;
};

void PushSprite(const Sprite& sprite, const Matrix4& transform);
void PushCircleTextured(const Matrix4& transform, const Texture& tex, const Vector4& tint = Vector4(1.0f));
void PushCircle(const Matrix4& transform, const Vector4& color);

} // namespace R2D
