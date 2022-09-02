#pragma once

#include "core/types.h"

struct InternalState;   // Defined based on the OS

struct PlatformState
{
    InternalState* internalState;
};

bool PlatformWindowStartup(PlatformState& pstate, const char* windowName, int x, int y, int width, int height, const char* iconPath);
void PlatformWindowShutdown(PlatformState& pstate);

bool PlatformPumpMessages();

// Memory Stuff

void* PlatformAllocate(u64 size);                   // TODO: Option for aligned memory
void* PlatformReallocate(void* block, u64 size);    // TODO: Option for aligned memory
void  PlatformFree(void* block);                    // TODO: Option for aligned memory

void* PlatformZeroMemory(void* block, u64 size);
void* PlatformCopyMemory(void* dest, const void* source, u64 size);
void* PlatformSetMemory(void* dest, s32 value, u64 size);

bool PlatformCompareMemory(const void* ptr1, const void* ptr2, u64 size);

// In Seconds
f64 PlatformGetTime();

// Input Things
void PlatformGetMousePosition(s32& x, s32& y);
void PlatformSetMousePosition(s32 x, s32 y);
void PlatformShowMouseCursor(bool value);

#ifdef GN_DEBUG

// Statistics
u64 PlatformGetMemoryAllocated();

#endif // GN_DEBUG