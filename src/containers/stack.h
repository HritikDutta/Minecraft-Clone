#pragma once

#include "core/logging.h"
#include "core/types.h"
#include "platform/platform.h"

template <typename T, bool isGlobal = false>
class Stack
{
private:
    static constexpr u64 START_CAP = 2;
    static constexpr f32 GROWTH_RATE = 1.5f;

public:
    // Getters
    inline u64 size()     const { return _size; }
    inline u64 capacity() const { return _capacity; }

    // Operators
    inline Stack& operator=(const Stack& other)
    {
        Clear();

        _stack = Reallocate(_stack, other._capacity);
        _capacity = other._capacity;
        _size = other._size;

        CopyStack(other._stack, other._size);

        return *this;
    }

    inline Stack& operator=(Stack&& other)
    {
        _stack = other._stack;
        _size  = other._size;
        _capacity = other._capacity;

        other._size = other._capacity = 0;
        other._stack = nullptr;

        return *this;
    }

    // Adding/Removing Elements
    inline T& Push(const T& value)
    {
        if (_size >= _capacity)
        {
            _stack = Reallocate(_stack, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }
        
        _stack[_size] = value;
        return _stack[_size++];
    }
    
    inline T& Push(T&& value)
    {
        if (_size >= _capacity)
        {
            _stack = Reallocate(_stack, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }
        
        _stack[_size] = std::move(value);
        return _stack[_size++];
    }

    template <typename... Args>
    inline T& Emplace(Args&&... args)
    {
        if (_size >= _capacity)
        {
            _stack = Reallocate(_stack, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }
        
        new(_stack + _size) T(std::forward<Args>(args)...);
        return _stack[_size++];
    }

    inline T Pop()
    {
        AssertWithMessage(_size > 0, "Trying to pop elements out of an empty stack!");
        return std::move(_stack[--_size]);
    }

    inline const T& Top() const
    {
        AssertWithMessage(_size > 0, "Trying to access top element of an empty stack!");
        return _stack[_size - 1];
    }

    inline T& Top()
    {
        AssertWithMessage(_size > 0, "Trying to access top element of an empty stack!");
        return _stack[_size - 1];
    }

    inline void Clear()
    {
        for (u64 i = 0; i < _size; i++)
            _stack[i].~T();
        
        _size = 0;
    }

    inline void Resize(u64 newCap)
    {
        _stack = Reallocate(_stack, newCap);
        _capacity = newCap;
    }

    // Constructors and Destructors
    Stack(u64 startCapacity = START_CAP)
    :   _stack(Allocate(startCapacity))
    ,   _size(0), _capacity(startCapacity)
    {
    }

    Stack(const std::initializer_list<T> list)
    :   _stack(Allocate(list.size()))
    ,   _size(0), _capacity(list.size())
    {
        for (auto value : list)
            new(_stack + (_size++)) T(value);
    }

    Stack(const Stack& other)
    :   _stack(Allocate(other._capacity)),
    ,   _size(other._size), _capacity(other._capacity)
    {
        CopyStack(other._stack, other._size);
    }

    Stack(Stack&& other)
    :   _stack(other._stack),
    :   _size(other._size), _capacity(other._capacity)
    {
        other._size = other._capacity = 0;
        other._stack = nullptr;
    }

    ~Stack()
    {
        if (isGlobal)
            return;

        if (!_stack)
            return;
        
        for (u64 i = 0; i < _size; i++)
            _stack[i].~T();

        Deallocate(_stack);

        _size = _capacity = 0;  // Not needed
        _stack = nullptr;
    }

private:
    static inline T* Allocate(u64 elements)
    {
        T* ptr = (T*) PlatformAllocate(elements * sizeof(T));
        AssertWithMessage(ptr, "Couldn't allocate stack.");
        return ptr;
    }
    
    static inline T* Reallocate(T* stack, u64 elements)
    {
        T* ptr = (T*) PlatformReallocate(stack, elements * sizeof(T));
        AssertWithMessage(ptr != nullptr, "Couldn't reallocate stack.");
        return ptr;
    }

    static inline void Deallocate(T* stack)
    {
        PlatformFree(stack);
    }

    inline void CopyStack(const T* stack, u64 size)
    {
        AssertWithMessage(_capacity >= size, "Trying to copy elements from larger stack!");

        for (u64 i = 0; i < size; i++)
            _stack[i] = stack[i];
    }

private:
    T*  _stack;
    u64 _size;
    u64 _capacity;
};