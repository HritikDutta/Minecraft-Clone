#include "graphics.h"

#ifdef GN_USE_OPENGL

#ifdef GN_PLATFORM_WINDOWS

#include "platform/platform.h"
#include "platform/internal/internal_win32.h"
#include "core/logging.h"

#include <iostream>
#include <windows.h>
// #include <windowsx.h>

// To load opengl functions
#include <glad/glad.h>
#include <wglext.h>

typedef BOOL (*SwapIntervalFunction)(int interval);
static SwapIntervalFunction internalSetSwapInterval;

static void* GLGetProcAddress(const char* name)
{
    void* ptr = (void*) wglGetProcAddress(name);

    if (ptr == nullptr || ptr == (void*) 0x1 ||
        ptr == (void*) 0x2 || ptr == (void*) 0x3 ||
        ptr == (void*) -1)
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        ptr = (void*) GetProcAddress(module, name);
    }

    return ptr;
}

#ifdef GN_DEBUG
static void APIENTRY GLDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                   GLsizei length, const char* message, const void* userParam)
{

    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    
    std::cout << std::endl;

    if (severity == GL_DEBUG_SEVERITY_HIGH)
        DebugBreak();
    
}
#endif // GN_DEBUG

bool GraphicsInit(InternalState& state)
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;     // TODO: Maybe some way of setting this?
    pfd.cColorBits = 32;                // TODO: Maybe some way of setting this?
    pfd.cDepthBits = 16;                // TODO: Maybe some way of setting this?
    pfd.iLayerType = PFD_MAIN_PLANE;

    state.hdc = GetDC(state.hwnd);

    if (!state.hdc)
    {
        MessageBoxA(state.hwnd, "Couldn't create device context for OpenGL!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Choose pixel format with the desired settings
    GLuint pixelFormat = ChoosePixelFormat(state.hdc, &pfd);
    if (!pixelFormat)
    {
        MessageBoxA(state.hwnd, "Couldn't find a suitable pixel format!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Set the pixel format for the program
    if (!SetPixelFormat(state.hdc, pixelFormat, &pfd))
    {
        MessageBoxA(state.hwnd, "Couldn't set the pixel format!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // Create Rendering Context
    HGLRC hrc = wglCreateContext(state.hdc);
    if (!hrc)
    {
        MessageBoxA(state.hwnd, "Couldn't create rendering context for OpenGL!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        wglDeleteContext(hrc);
        return false;
    }

    // Activate the rendering context for OpenGL
    if (!wglMakeCurrent(state.hdc, hrc))
    {
        MessageBoxA(state.hwnd, "Couldn't activate the rendering context for OpenGL!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        wglDeleteContext(hrc);
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc) GLGetProcAddress))
    {
        MessageBoxA(state.hwnd, "Couldn't load OpenGL functios!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        wglDeleteContext(hrc);
        return false;
    }

    if (!gladLoadGL())
    {
        MessageBoxA(state.hwnd, "Couldn't load OpenGL functios!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        wglDeleteContext(hrc);
        return false;
    }

    // TODO: Need a better way to do this (Set Vsync)
    internalSetSwapInterval = (SwapIntervalFunction) GLGetProcAddress("wglSwapIntervalEXT");

    // Initialize opengl with things like 4x MSAA
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) GLGetProcAddress("wglChoosePixelFormatARB");
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) GLGetProcAddress("wglCreateContextAttribsARB");

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB)
    {
        Warn("Couldn't find wglCreateContextAttribsARB function!");
        return true;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hrc);

    int iPixelAttribs[] =
    {
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,    // Must support OGL rendering
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,    // pf that can run a window
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,    // must be HW accelerated
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,     // Double buffered context

        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,             // 8 bits of each R, G and B
        WGL_DEPTH_BITS_ARB, 16,             // 24 bits of depth precision for window
        WGL_STENCIL_BITS_ARB, 8, // Template buffer, template buffer number = 8

        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE, // MSAA ON, open multiple samples
        WGL_SAMPLES_ARB, 4,              // ​​4X MSAA, multiple sample samples are 4

        0
    };

    pixelFormat = -1;
    u32 pixelCount = 0;
    f32 fPixelAttribs[] = { 0, 0 };

    wglChoosePixelFormatARB(state.hdc, iPixelAttribs, fPixelAttribs, 1, (int*) &pixelFormat, &pixelCount);
    if (pixelFormat == -1)
    {
        MessageBoxA(state.hwnd, "Couldn't find a suitable pixel format!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

#if GN_DEBUG
    s32 debugBit = WGL_CONTEXT_DEBUG_BIT_ARB;
#else
    s32 debugBit = 0;
#endif // GN_DEBUG

    s32 attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | debugBit,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    hrc = wglCreateContextAttribsARB(state.hdc, nullptr, attribs);
    if (!hrc)
    {
        MessageBoxA(state.hwnd, "Couldn't create rendering context for OpenGL!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        wglDeleteContext(hrc);
        return false;
    }

    if (!wglMakeCurrent(state.hdc, hrc))
    {
        MessageBoxA(state.hwnd, "Couldn't activate the rendering context for OpenGL!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        wglDeleteContext(hrc);
        return false;
    }

    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Opengl Settings?
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);

#ifdef GN_DEBUG
    // Setup debugging for opengl
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
                                0, nullptr, GL_TRUE);

        std::cout << "[OpenGL] Ready to debug..." << std::endl;
    }
#endif // GN_DEBUG

    return true;
}

void GraphicsShutdown(InternalState& state)
{
    wglMakeCurrent(state.hdc, NULL);
    wglDeleteContext(wglGetCurrentContext());
}

void GraphicsSwapBuffers(const PlatformState& pstate)
{
    SwapBuffers(pstate.internalState->hdc);
}

void GraphicsResizeCanvasCallback(s32 width, s32 height)
{
    glViewport(0, 0, width, height);
}

void GraphicsSetVsync(bool value)
{
    internalSetSwapInterval((int) value);
}

void GraphicsSetClearColor(f32 red, f32 green, f32 blue, f32 alpha)
{
    glClearColor(red, green, blue, alpha);
}

void GraphicsClearCanvas()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

#endif // GN_PLATFORM_WINDOWS

#endif // GN_USE_OPENGL