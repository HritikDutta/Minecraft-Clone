#pragma once

#include "types.h"

extern inline void InputGetState(Application& app);
extern inline void InputStateUpdate(Application& app);

extern inline void InputProcessKey(Key key, bool pressed);
extern inline void InputProcessMouseButton(MouseButton btn, bool pressed);
extern inline void InputProcessMouseWheel(s32 z);