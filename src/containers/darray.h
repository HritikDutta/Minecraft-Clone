#pragma once

#include "core/logging.h"
#include "core/types.h"
#include "platform/platform.h"

template <typename T>
class DynamicArray
{
private:
    static constexpr u64 START_CAP   = 2;
    static constexpr f32 GROWTH_RATE = 1.5f;

public:
    // C++11 Iterators

    // These are meant to be used temporarily. DON'T store them in
    // variables to reference elements in the set outside a loop.
    class iterator
    {
    public:
        // Operators

        // Pre Increment (++it)
        inline iterator& operator++(int)
        {
            advance();
            return *this;
        }

        // Post Increment (it++)
        inline iterator operator++()
        {
            iterator it = *this;
            advance();
            return it;
        }

        inline T& operator*()
        {
            AssertWithMessage(_index <= _array->_size, "Trying to dereference a non existant value!");
            return _array->_array[_index];
        }

        inline const T& operator*() const
        {
            AssertWithMessage(_index <= _array->_size, "Trying to dereference a non existant value!");
            return _array->_array[_index];
        }

        inline bool operator==(const iterator& other) const
        {
            return _array == other._array &&
                   _index == other._index;
        }

        inline bool operator!=(const iterator& other) const
        {
            return _array != other._array ||
                   _index != other._index;
        }

        // Conversions
        inline operator bool() const
        {
            return *this != _array->end();
        }

        // Getters
        inline u64 index() const
        {
            return _index;
        }

        // Constructors
        iterator(const DynamicArray* array, u64 index)
        :   _array(array), _index(index)
        {
        }
    
    private:
        inline void advance()
        {
            if (_index > _array->_size)
                return;
            
            _index++;
        }

    private:
        const DynamicArray* _array;
        u64 _index;
    };
    
    inline const iterator begin() const { return iterator(this, 0); }
    inline       iterator begin()       { return iterator(this, 0); }

    inline const iterator end() const { return iterator(this, _size); }
    inline       iterator end()       { return iterator(this, _size); }

public:
    // Getters
    inline u64 size()     const { return _size; }
    inline u64 capacity() const { return _capacity; }

    inline const T* data() const { return _array; };
    inline       T* data()       { return _array; };

    // Operators
    inline const T& operator[](u64 index) const
    {
        AssertWithMessage(index < _size, "Trying to get to index larger than size!");
        return _array[index];
    }

    inline T& operator[](u64 index)
    {
        AssertWithMessage(index < _size, "Trying to get to index larger than size!");
        return _array[index];
    }

    inline DynamicArray& operator=(const DynamicArray& other)
    {
        Clear();

        _array = Reallocate(_array, other._capacity);
        _capacity = other._capacity;
        _size = other._size;

        CopyArray(other._array, other._size);

        return *this;
    }

    inline DynamicArray& operator=(DynamicArray&& other)
    {
        _array = other._array;
        _size  = other._size;
        _capacity = other._capacity;

        other._size = other._capacity = 0;
        other._array = nullptr;

        return *this;
    }

    // Explicit Functions
    inline void ManualInit(u64 startCapacity = START_CAP)
    {
        _array = Allocate(startCapacity);
        _capacity = startCapacity;
        _size = 0;
    }

    // Adding/Removing Elements
    inline T& PushBack(const T& value)
    {
        if (_size >= _capacity)
        {
            _array = Reallocate(_array, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }
        
        _array[_size] = value;
        return _array[_size++];
    }
    
    inline T& PushBack(T&& value)
    {
        if (_size >= _capacity)
        {
            _array = Reallocate(_array, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }
        
        _array[_size] = std::move(value);
        return _array[_size++];
    }

    template <typename... Args>
    inline T& EmplaceBack(Args&&... args)
    {
        if (_size >= _capacity)
        {
            _array = Reallocate(_array, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }
        
        new(_array + _size) T(std::forward<Args>(args)...);
        return _array[_size++];
    }

    inline T PopBack()
    {
        AssertWithMessage(_size > 0, "Trying to pop elements out of an empty array!");
        return std::move(_array[--_size]);
    }

    inline T& Insert(u64 index, const T& val)
    {
        if (_size >= _capacity)
        {
            _array = Reallocate(_array, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }

        for (u64 i = _size; i > index; i--)
            _array[i] = std::move(_array[i - 1]);

        _array[index] = val;
        return _array[index];
    }

    inline T& Insert(u64 index, T&& val)
    {
        if (_size >= _capacity)
        {
            _array = Reallocate(_array, _capacity * GROWTH_RATE);
            _capacity = _capacity * GROWTH_RATE;
        }

        for (u64 i = _size; i > index; i--)
            _array[i] = std::move(_array[i - 1]);

        _array[index] = std::move(val);
        return _array[index];
    }

    inline void EraseAt(u64 index)
    {
        AssertWithMessage(index < _size, "Trying to erase at index higher than size!");

        _array[index].~T();
        _size--;

        for (u64 i = index; i < _size; i++)
            _array[index] = std::move(_array[index + 1]);
    }

    inline void EraseSwap(u64 index)
    {
        AssertWithMessage(index < _size, "Trying to erase at index higher than size!");

        _array[index].~T();
        _size--;

        _array[index] = std::move(_array[_size]);
    }

    inline void Clear(bool callDestructors = true)
    {
        if (callDestructors)
        {
            for (u64 i = 0; i < _size; i++)
                _array[i].~T();
        }
        
        _size = 0;
    }

    // Memory Stuff
    inline void Reserve(u64 capacity)
    {
        _array = Reallocate(_array, capacity);
        _capacity = capacity;
        _size = (_size < capacity) ? _size : capacity;
    }
    
    inline void Resize(u64 newSize)
    {
        _array = Reallocate(_array, newSize);
        _capacity = newSize;
        _size = newSize;
    }

    inline void Free()
    {
        if (!_array)
            return;

        for (u64 i = 0; i < _size; i++)
            _array[i].~T();

        Deallocate(_array);

        _size = _capacity = 0;  // Not needed
        _array = nullptr;
    }

    // Constructors and Destructors
    DynamicArray(u64 startCapacity = START_CAP)
    :   _array(Allocate(startCapacity))
    ,   _size(0), _capacity(startCapacity)
    {
    }

    DynamicArray(const std::initializer_list<T> list)
    :   _array(Allocate(list.size()))
    ,   _size(0), _capacity(list.size())
    {
        for (auto value : list)
            new(_array + (_size++)) T(value);
    }

    DynamicArray(const DynamicArray& other)
    :   _array(Allocate(other._capacity))
    ,   _size(other._size), _capacity(other._capacity)
    {
        CopyArray(other._array, _size);
    }

    DynamicArray(DynamicArray&& other)
    :   _array(other._array),
    ,   _size(other._size), _capacity(other._capacity)
    {
        other._size = other._capacity = 0;
        other._array = nullptr;
    }

    ~DynamicArray()
    {
        Free();
    }

private:
    static inline T* Allocate(u64 elements)
    {
        T* ptr = (T*) PlatformAllocate(elements * sizeof(T));
        AssertWithMessage(ptr, "Couldn't allocate array.");
        return ptr;
    }

    static inline T* Reallocate(T* _array, u64 elements)
    {
        T* ptr = (T*) PlatformReallocate(_array, elements * sizeof(T));
        AssertWithMessage(ptr != nullptr, "Couldn't reallocate array.");
        return ptr;
    }

    static inline void Deallocate(T* array)
    {
        PlatformFree(array);
    }

    inline void CopyArray(const T* array, u64 size)
    {
        AssertWithMessage(_capacity >= size, "Trying to copy elements from larger array!");

        for (u64 i = 0; i < size; i++)
            _array[i] = array[i];
    }

private:
    T*  _array;
    u64 _size;
    u64 _capacity;
};