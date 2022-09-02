#pragma once

#include "core/types.h"
#include "math/math.h"
#include "core/application.h"
#include "containers/stringview.h"
#include "containers/hashtable.h"
#include "graphics/texture.h"

namespace Imgui
{

// Initialization and Shutdown
void Init(const Application& app);
void Shutdown();

// Begin and End UI Pass
void Begin();
void End();

struct ID
{
    s32 primary;
    s32 secondary;

    inline bool operator==(const ID& other)
    {
        return primary == other.primary &&
                secondary == other.secondary;
    }

    inline bool operator!=(const ID& other)
    {
        return primary != other.primary ||
                secondary != other.secondary;
    }
};

struct Rect
{
    Vector3 topLeft;
    Vector2 size;
};

using Image = Texture;
using ImageSettings = TextureSettings;

struct Font
{
    struct GlyphData
    {
        f32 advance;
        Vector4 planeBounds;
        Vector4 atlasBounds;
    };

    Texture texture;

    f32 lineHeight;
    f32 ascender, descender;
    u32 size;

    GlyphData glyphs[127 - ' '];
    HashTable<s32, f32> kerningTable;

    void Load(StringView atlaspath, StringView datapath);
    void Free();
};

Vector2 GetRenderedTextSize(StringView text, Font& font, f32 size = -1.0f);
Vector2 GetRenderedCharSize(char ch, Font& font, f32 size = -1.0f);

void RenderRect(const Rect& rect, const Vector4& color);
void RenderImage(const Image& image, const Vector3& topLeft, const Vector2& scale = Vector2(1.0f), const Vector4& tint = Vector4(1.0f));
bool RenderButton(ID id, const Rect& rect, const Vector4& defaultColor = Vector4(0.5f, 0.5f, 0.5f, 1.0f), const Vector4& hoverColor = Vector4(0.75f, 0.75f, 0.75f, 1.0f), const Vector4& pressedColor = Vector4(0.35f, 0.35f, 0.35f, 1.0f));

// (size < 0 would simply use the default font size)
void RenderText(StringView text, Font& font, const Vector3& topLeft, f32 size = -1.0f, const Vector4& tint = Vector4(1.0f));
void RenderChar(char ch, Font& font, const Vector3& topLeft, f32 size = -1.0f, const Vector4& tint = Vector4(1.0f));

} // namespace Imgui

#define GenImguiID() (Imgui::ID { __LINE__, 0 })
#define GenImguiIDWithSecondary(sec) (Imgui::ID { __LINE__, (sec) })