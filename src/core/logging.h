#pragma once

#include <iostream>

#ifndef GN_RELEASE

inline static void Assert_Internal(const char* file, const char* function, const int line, const char* msg)
{
    std::cout << "ASSERTION FAILED: " << msg << '\n';
    std::cout << "File: " << file << '\n';
    std::cout << "Function: " << function << '\n';
    std::cout << "Line: " << line << '\n';
}

inline static void Warn_Internal(const char* file, const char* function, const int line, const char* msg)
{
    std::cout << "WARNING: " << msg << '\n';
    std::cout << "File: " << file << '\n';
    std::cout << "Function: " << function << '\n';
    std::cout << "Line: " << line << '\n';
}

// Defining a compiler agnostic way for haulting the program
#ifdef _MSC_VER
#include <intrin.h>
#define DebugBreak() __debugbreak()
#else
#define DebugBreak() __builtin_trap()
#endif

#define AssertWithMessage(x, msg)  if (!(x)) { Assert_Internal(__FILE__, __FUNCSIG__, __LINE__, msg); DebugBreak(); }
#define Assert(x)  if (!(x)) { Assert_Internal(__FILE__, __FUNCSIG__, __LINE__, #x); DebugBreak(); }
#define AssertNotImplemented() { Assert_Internal(__FILE__, __FUNCSIG__, __LINE__, "Function not implemented!"); DebugBreak(); }

#define WarnIf(cond, msg) if ((cond)) { Warn_Internal(__FILE__, __FUNCSIG__, __LINE__, msg); }
#define Warn(msg) Warn_Internal(__FILE__, __FUNCSIG__, __LINE__, msg)

#else

#define AssertWithMessage(x, msg) (x)
#define Assert(x) (x)
#define AssertNotImplemented()

#define WarnIf(cond, msg)
#define Warn(msg)

#endif // GN_RELEASE