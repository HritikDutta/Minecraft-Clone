#pragma once

#include "core/types.h"
#include "containers/stringview.h"
#include "containers/string.h"
#include "containers/hashtable.h"
#include "math/math.h"

struct Shader
{
    enum class Type
    {
        FRAGMENT_SHADER,
        VERTEX_SHADER,
        NUM_TYPES
    };

    bool CompileFromFile(StringView filepath, Type type);
    bool CompileSource(StringView source, Type type);
    bool Link();

    void Bind() const;

    void SetUniform1f(StringView uniformName, f32 value);
    void SetUniform1i(StringView uniformName, s32 value);
    void SetUniform1iv(StringView uniformName, u32 count, s32* data);

    void SetUniform2f(StringView uniformName, f32 v0, f32 v1);
    void SetUniform2fv(StringView uniformName, int count, f32* vs);

    void SetUniform3f(StringView uniformName, f32 v0, f32 v1, f32 v2);
    void SetUniform3fv(StringView uniformName, int count, f32* vs);

    void SetUniform4f(StringView uniformName, f32 v0, f32 v1, f32 v2, f32 v3);
    void SetUniform4fv(StringView uniformName, int count, f32* vs);

    void SetUniformMatrix4(StringView uniformName, const Matrix4& mat);

    u32 shaderIDs[(u64) Type::NUM_TYPES];
    u32 program;
    HashTable<String, s32> uniformLocations;
};