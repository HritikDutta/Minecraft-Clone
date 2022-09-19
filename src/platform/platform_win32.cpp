#include "platform.h"

#ifdef GN_PLATFORM_WINDOWS

#include "core/types.h"
#include "core/input.h"
#include "core/input_processing.h"
#include "core/application_internal.h"
#include "core/logging.h"
#include "internal/internal_win32.h"
#include "graphics/graphics.h"

#include <windows.h>
#include <windowsx.h>   // For param input extraction
#include <psapi.h>      // For memory usage data

// Clock Stuff
static f64 clockFrequency;
static LARGE_INTEGER startTime;

#ifdef GN_DEBUG
// Process Handle for Memory Usage Stats
static HANDLE processHandle = 0;
#endif // GN_DEBUG

LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam);

bool PlatformWindowStartup(PlatformState& pstate, const char* windowName, int x, int y, int width, int height, const char* iconPath)
{
    pstate.internalState = (InternalState*) PlatformAllocate(sizeof(InternalState));
    InternalState& state = *pstate.internalState;

    state.hInstance = GetModuleHandleA(0);

    // Setup and Register Window Class
    HICON icon = LoadIcon(state.hInstance, IDI_APPLICATION);
    if (iconPath)
        HICON icon = (HICON) LoadImageA(state.hInstance, iconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

    WNDCLASSA wc = {};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = Win32ProcessMessage;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state.hInstance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   // NULL; Manage the cursor manually
    wc.hbrBackground = NULL;                    // Transparent
    wc.lpszClassName = "Gonad Window Class";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window Registration Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 windowExStyle = WS_EX_APPWINDOW;

    // This is prone to change
    windowStyle |= WS_MAXIMIZEBOX;
    windowStyle |= WS_MINIMIZEBOX;
    windowStyle |= WS_THICKFRAME;

    // Get size of window
    RECT borderRect = { 0 };
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

    // Adjust border size and position
    u32 windowX = x + borderRect.left;
    u32 windowY = y + borderRect.top;
    u32 windowWidth = width + (borderRect.right - borderRect.left);
    u32 windowHeight = height + (borderRect.bottom - borderRect.top);

    HWND handle = CreateWindowExA(windowExStyle, "Gonad Window Class", windowName,
                                  windowStyle, windowX, windowY, windowWidth, windowHeight,
                                  0, 0, state.hInstance, 0);

    if (!handle)
    {
        MessageBoxA(NULL, "Window creation failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    state.hwnd = handle;

    SetWindowLongPtrA(state.hwnd, GWLP_USERDATA, (LONG_PTR) &pstate);

    if (!GraphicsInit(state))
    {
        // Error has been thrown by the function already
        return false;
    }

    // Show the window
    s32 showWindowCommandFlags = SW_SHOW;
    // This should be SW_MAXIMIZE for maximizing at start
    ShowWindow(state.hwnd, showWindowCommandFlags);

    // Initialize Clock
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (f64) frequency.QuadPart;
    QueryPerformanceCounter(&startTime);

#ifdef GN_DEBUG
    // Get Process Handle for Memory Usage Stats
    DWORD processID = GetCurrentProcessId();
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION |
                                PROCESS_VM_READ,
                                FALSE, processID);
#endif // GN_DEBUG

    return true;
}

void PlatformWindowShutdown(PlatformState& pstate)
{
    InternalState& state = *pstate.internalState;

    if (state.hwnd)
    {
#ifdef GN_DEBUG
        if (processHandle)
            CloseHandle(processHandle);
#endif // GN_DEBUG

        GraphicsShutdown(state);
        DestroyWindow(state.hwnd);
        state.hwnd = 0;
    }
}

bool PlatformPumpMessages()
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
}

void* PlatformAllocate(u64 size)
{
    return malloc(size);
}

void* PlatformReallocate(void* block, u64 size)
{
    return realloc(block, size);
}

void PlatformFree(void* block)
{
    free(block);
}

void* PlatformZeroMemory(void* block, u64 size)
{
    return memset(block, 0, size);
}

void* PlatformCopyMemory(void* dest, const void* source, u64 size)
{
    return memcpy(dest, source, size);
}

void* PlatformSetMemory(void* block, s32 value, u64 size)
{
    return memset(block, value, size);
}

bool PlatformCompareMemory(const void* ptr1, const void* ptr2, u64 size)
{
    return memcmp(ptr1, ptr2, size) == 0;
}

f64 PlatformGetTime()
{
    LARGE_INTEGER nowTime;
    QueryPerformanceCounter(&nowTime);
    return (f64) (nowTime.QuadPart - startTime.QuadPart) * clockFrequency; 
}

#ifdef GN_DEBUG
u64 PlatformGetMemoryAllocated()
{
    if (!processHandle)
        return 0;

    u64 mem = 0;

    // Get information about the memory usage of the process.
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
        mem = pmc.PrivateUsage;

    return mem;
}
#endif // GN_DEBUG

LRESULT CALLBACK Win32ProcessMessage(HWND hwnd, u32 msg, WPARAM wParam, LPARAM lParam)
{
    PlatformState* pstate = (PlatformState*) GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg)
    {
        case WM_ERASEBKGND:
            // Notify the OS that the erasing will be handled by the app to prevent flickering
            return 1;
        
        case WM_CLOSE:
        {
            if (pstate)
                ApplicationExit();
        }
        return 0;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        
        case WM_SIZE:
        {
            if (pstate)
            {
                s32 width = LOWORD(lParam);
                s32 height = HIWORD(lParam);

                ApplicationWindowResizeCallback(width, height);
                GraphicsResizeCanvasCallback(width, height);
            }
        } return 0;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            bool pressed = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
            Key key = (Key) wParam;
            InputProcessKey(key, pressed);
        } break;

        case WM_MOUSEWHEEL:
        {
            s32 zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

            if (zDelta != 0) {
                // Flatten the input to an OS-independent (-1, 1)
                zDelta = (zDelta < 0) ? -1 : 1;
                InputProcessMouseWheel(zDelta);
            }
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            bool pressed = msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK ||
                           msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK ||
                           msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK;
            MouseButton btn = MouseButton::NUM_BUTTONS;

            switch (msg)
            {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_LBUTTONDBLCLK:
                {
                    btn = MouseButton::LEFT;
                } break;

                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MBUTTONDBLCLK:
                {
                    btn = MouseButton::MIDDLE;
                } break;

                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_RBUTTONDBLCLK:
                {
                    btn = MouseButton::RIGHT;
                } break;
            }

            InputProcessMouseButton(btn, pressed);
        } break;

        case WM_KILLFOCUS:
        case WM_SETFOCUS:
        {
            Application& app = GetActiveApplication();
            app.window.hasFocus = msg == WM_SETFOCUS;
        } break;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void PlatformGetMousePosition(s32& x, s32& y)
{
    POINT point;
    GetCursorPos(&point);

    x = point.x;
    y = point.y;
}

void PlatformSetMousePosition(s32 x, s32 y)
{
    SetCursorPos(x, y);
}

void PlatformShowMouseCursor(bool value)
{
    ShowCursor(value);
}

#endif // GN_PLATFORM_WINDOWS