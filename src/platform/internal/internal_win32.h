#pragma once

#ifdef GN_PLATFORM_WINDOWS

#include <windows.h>

struct InternalState
{
    HINSTANCE hInstance;
    HWND hwnd;
    HDC hdc;
};

#endif // GN_PLATFORM_WINDOWS