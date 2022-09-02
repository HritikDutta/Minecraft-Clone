#pragma once

#include "core/logging.h"
#include "core/types.h"
#include "core/utils.h"
#include "platform/platform.h"

template <typename T>
struct Array3D
{
public:
    void Allocate(u32 dimension)
    {
        _buffer = (T*) PlatformAllocate(dimension * dimension * dimension * sizeof(T));
        AssertWithMessage(_buffer != nullptr, "Couldn't allocate 3d array!");
        _dimension = dimension;
    }

    void Free()
    {
        PlatformFree(_buffer);
        _dimension = 0;
    }

    void SwapWith(Array3D& other)
    {
        Swap(_buffer, other._buffer);
        Swap(_dimension, other._dimension);
    }
    
    const T& at(u32 x, u32 y, u32 z) const
    {
        u32 index = x + y * _dimension + z * _dimension * _dimension;
        return _buffer[index];
    }

    T& at(u32 x, u32 y, u32 z)
    {
        u32 index = x + y * _dimension + z * _dimension * _dimension;
        return _buffer[index];
    }

    const T* data() const { return _buffer; }
    T* data() { return _buffer; }

    u32 totalSize() const { return _dimension * _dimension * _dimension; }
    u32 dimension() const { return _dimension; }

private:
    T*  _buffer = nullptr;
    u32 _dimension;
};