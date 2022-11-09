#pragma once

#include "input.h"
#include "input_processing.h"
#include "types.h"
#include "application_internal.h"
#include "platform/platform.h"
#include "containers/darray.h"

struct KeyboardState
{
    bool keys[(int) Key::NUM_KEYS];
};

struct MouseState
{
    s32 x, y;
    bool buttons[(int) MouseButton::NUM_BUTTONS];
    bool centerCursor = false;
};

struct InputState
{
    KeyboardState keyboardState;
    MouseState mouseState;
};

struct InputEvents
{
    DynamicArray<KeyDownCallback> keyDownCallbacks;
    DynamicArray<MouseScrollCallback> mouseScrollCallbacks;
};

static InputState currentInputState  = {};
static InputState previousInputState = {};
static InputEvents inputEvents;

static bool hadFocus = true;

static inline s32 RoundToLowerEven(s32 num)
{
    return num - (num % 2);
}

inline void InputGetState(Application& app)
{
    // IDK if this is a good solution or not...
    s32 width  = RoundToLowerEven(app.window.width);
    s32 height = RoundToLowerEven(app.window.height);

    // Update current mouse position
    if (hadFocus)
        PlatformGetMousePosition(currentInputState.mouseState.x, currentInputState.mouseState.y);

    if (app.window.hasFocus && currentInputState.mouseState.centerCursor)
        PlatformSetMousePosition(width / 2, height / 2);
}

inline void InputStateUpdate(Application& app)
{
    PlatformCopyMemory(&previousInputState, &currentInputState, sizeof(InputState));

    if (currentInputState.mouseState.centerCursor)
    {
        previousInputState.mouseState.x = app.window.width  / 2;
        previousInputState.mouseState.y = app.window.height / 2;
    }

    hadFocus = app.window.hasFocus;
}

inline void InputProcessKey(Key key, bool pressed)
{
    if (pressed && !currentInputState.keyboardState.keys[(int) key])
    {
        for (int i = 0; i < inputEvents.keyDownCallbacks.size(); i++)
            inputEvents.keyDownCallbacks[i](GetActiveApplication(), key);
    }

    currentInputState.keyboardState.keys[(int) key] = pressed;
}

inline void InputProcessMouseButton(MouseButton btn, bool pressed)
{
    currentInputState.mouseState.buttons[(int) btn] = pressed;
}

inline void InputProcessMouseWheel(s32 z)
{
    for (int i = 0; i < inputEvents.mouseScrollCallbacks.size(); i++)
        inputEvents.mouseScrollCallbacks[i](GetActiveApplication(), z);
}

// Implementations for input.h

bool Input::GetKey(Key key)
{
    return currentInputState.keyboardState.keys[(int) key];
}

bool Input::GetKeyDown(Key key)
{
    return currentInputState.keyboardState.keys[(int) key] &&
           !previousInputState.keyboardState.keys[(int) key];
}

bool Input::GetKeyUp(Key key)
{
    return !currentInputState.keyboardState.keys[(int) key] &&
           previousInputState.keyboardState.keys[(int) key];
}

bool Input::GetMouseButton(MouseButton button)
{
    return currentInputState.mouseState.buttons[(int) button];
}

bool Input::GetMouseButtonDown(MouseButton button)
{
    return currentInputState.mouseState.buttons[(int) button] &&
           !previousInputState.mouseState.buttons[(int) button];
}

bool Input::GetMouseButtonUp(MouseButton button)
{
    return !currentInputState.mouseState.buttons[(int) button] &&
           previousInputState.mouseState.buttons[(int) button];
}

Vector2 Input::MousePosition()
{
    Application& app = GetActiveApplication();
    
    return Vector2(
        app.window.refWidth  * currentInputState.mouseState.x / app.window.width,
        app.window.refHeight * currentInputState.mouseState.y / app.window.height
    );
}

Vector2 Input::DeltaMousePosition()
{
    Application& app = GetActiveApplication();

    s32 delX = currentInputState.mouseState.x - previousInputState.mouseState.x;
    s32 delY = currentInputState.mouseState.y - previousInputState.mouseState.y;
    
    return Vector2(
        app.window.refWidth  * delX / app.window.width,
        app.window.refHeight * delY / app.window.height
    );
}

void Input::RegisterKeyDownEventCallback(KeyDownCallback callback)
{
    inputEvents.keyDownCallbacks.EmplaceBack(callback);
}

void Input::RegisterMouseScrollEventCallback(MouseScrollCallback callback)
{
    inputEvents.mouseScrollCallbacks.EmplaceBack(callback);
}

void Input::CenterMouse(bool value)
{
    currentInputState.mouseState.centerCursor = value;
}