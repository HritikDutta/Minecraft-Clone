#include "shader.h"

#include "core/types.h"
#include "core/logging.h"
#include "containers/stringview.h"
#include "containers/string.h"
#include "containers/hashtable.h"
#include "math/math.h"
#include "fileio/fileio.h"

#include <glad/glad.h>

bool Shader::CompileFromFile(StringView filepath, Type type)
{
    // Since I just know fragment and vertex shaders, this works lol
    GLenum glShaderType = GL_FRAGMENT_SHADER + (int) type;

    u32 shader = glCreateShader(glShaderType);

    // Load shader file
    String source;
    LoadFileToString(filepath, source);

    const char* srcCstr = source.cstr();
    glShaderSource(shader, 1, &srcCstr, nullptr);
    glCompileShader(shader);

#ifdef GN_DEBUG

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    
    if (compileStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(shader, 1024, &logLength, message);
        std::cout << message << '\n';

        return false;
    }

#endif // GN_DEBUG

    shaderIDs[(int) type] = shader;
    return true;
}

bool Shader::CompileSource(StringView source, Type type)
{
    // Since I just know fragment and vertex shaders, this works lol
    GLenum glShaderType = GL_FRAGMENT_SHADER + (int) type;

    u32 shader = glCreateShader(glShaderType);

    const char* srcCstr = source.cstr();
    glShaderSource(shader, 1, &srcCstr, nullptr);
    glCompileShader(shader);

#ifdef GN_DEBUG

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    
    if (compileStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetShaderInfoLog(shader, 1024, &logLength, message);
        std::cout << message << '\n';

        return false;
    }

#endif // GN_DEBUG

    shaderIDs[(int) type] = shader;
    return true;
}

bool Shader::Link()
{
    program = glCreateProgram();
    glAttachShader(program, shaderIDs[0]);
    glAttachShader(program, shaderIDs[1]);
    glLinkProgram(program);

#ifdef GN_DEBUG

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        GLsizei logLength = 0;
        GLchar message[1024];
        glGetProgramInfoLog(program, 1024, &logLength, message);
        std::cout << message << '\n';

        return false;
    }

#endif

    glDeleteShader(shaderIDs[0]);
    glDeleteShader(shaderIDs[1]);

    return true;
}

void Shader::Bind() const
{
    glUseProgram(program);
}

static inline int GetUniformLocation(Shader* shader, StringView uniformName)
{
    return glGetUniformLocation(shader->program, uniformName.cstr());
    
    /* TODO: Need to fix HashTables first

    if (shader->uniformLocations.Find(uniformName))
        return shader->uniformLocations[uniformName];
        
    int uniformLocation = glGetUniformLocation(shader->program, uniformName.cstr());
    shader->uniformLocations[uniformName] = uniformLocation;
    return uniformLocation;

    */
}

void Shader::SetUniform1f(StringView uniformName, f32 value)
{
    glUniform1f(GetUniformLocation(this, uniformName), value);
}

void Shader::SetUniform1i(StringView uniformName, s32 value)
{
    glUniform1i(GetUniformLocation(this, uniformName), value);
}

void Shader::SetUniform1iv(StringView uniformName, u32 count, s32* data)
{
    glUniform1iv(GetUniformLocation(this, uniformName), count, data);
}

void Shader::SetUniform2f(StringView uniformName, f32 v0, f32 v1)
{
    glUniform2f(GetUniformLocation(this, uniformName), v0, v1);
}

void Shader::SetUniform2fv(StringView uniformName, int count, f32* vs)
{
    glUniform2fv(GetUniformLocation(this, uniformName), count, vs);
}

void Shader::SetUniform3f(StringView uniformName, f32 v0, f32 v1, f32 v2)
{
    glUniform3f(GetUniformLocation(this, uniformName), v0, v1, v2);
}

void Shader::SetUniform3fv(StringView uniformName, int count, f32* vs)
{
    glUniform3fv(GetUniformLocation(this, uniformName), count, vs);
}

void Shader::SetUniform4f(StringView uniformName, f32 v0, f32 v1, f32 v2, f32 v3)
{
    glUniform4f(GetUniformLocation(this, uniformName), v0, v1, v2, v3);
}

void Shader::SetUniform4fv(StringView uniformName, int count, f32* vs)
{
    glUniform4fv(GetUniformLocation(this, uniformName), count, vs);
}

void Shader::SetUniformMatrix4(StringView uniformName, const Matrix4& mat)
{
    glUniformMatrix4fv(GetUniformLocation(this, uniformName), 1, false, (f32*) mat.data);
}