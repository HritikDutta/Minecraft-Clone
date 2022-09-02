#pragma once

/*

Custom String Class.

This is a padded string class.
This means that the memory allocated for these strings
is always a multiple of 16 bytes.

This is done to take advantage of SSE instructions
that can work on 16 characters at a time.

Unlike the Dynamic Array, the growth of the buffer is
linear to save on space.

This string class also uses pooling.
This means that when a string buffer is freed, the
same buffer can be reused for other strings in the future.

*/

#include <ostream>
#include <emmintrin.h>

#include "core/types.h"
#include "core/logging.h"
#include "platform/platform.h"
#include "stack.h"
#include "math/common.h"

class String
{
private:
    static constexpr u64 _alignment = 16;

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

        inline char& operator*()
        {
            AssertWithMessage(_index <= _str->_length, "Trying to dereference a non existant value!");
            return _str->_buffer[_index];
        }

        inline const char& operator*() const
        {
            AssertWithMessage(_index <= _str->_length, "Trying to dereference a non existant value!");
            return _str->_buffer[_index];
        }

        inline bool operator==(const iterator& other) const
        {
            return _str == other._str &&
                   _index == other._index;
        }

        inline bool operator!=(const iterator& other) const
        {
            return _str != other._str ||
                   _index != other._index;
        }

        // Conversions
        inline operator bool() const
        {
            return *this != _str->end();
        }

        // Getters
        inline u64 index() const
        {
            return _index;
        }

        // Constructors
        iterator(const String* str, u64 index)
        :   _str(str), _index(index)
        {
        }
    
    private:
        inline void advance()
        {
            if (_index > _str->_length)
                return;
            
            _index++;
        }

    private:
        const String* _str;
        u64 _index;
    };
    
    inline const iterator begin() const { return iterator(this, 0); }
    inline       iterator begin()       { return iterator(this, 0); }

    inline const iterator end() const { return iterator(this, _length); }
    inline       iterator end()       { return iterator(this, _length); }

public:
    // Getters
    inline u64 size()     const { return _length; }
    inline u64 length()   const { return _length; }
    inline u64 capacity() const { return _capacity; }

    inline const char* cstr() const { return (char*) _buffer; };
    inline       char* cstr()       { return (char*) _buffer; };

    // Search Functions
    inline iterator FindFirstOf(char ch) const
    {
        __m128i needle = _mm_set1_epi8(ch);
        u64 batches = _capacity / _alignment;

        for (u64 i = 0; i < batches; i++)
        {
            u16 equalsMask = _mm_movemask_epi8(_mm_cmpeq_epi8(_sse[i], needle));
            if (equalsMask == 0)
                continue;
            
            u16 minBit = equalsMask & ~(equalsMask - 1);
            u64 offset = log2(minBit);

            return iterator(this, (i * _alignment) + offset);
        }

        return end();
    }

    inline iterator FindLastOf(char ch) const
    {
        __m128i needle = _mm_set1_epi8(ch);
        u64 batches = _capacity / _alignment;

        for (u64 i = batches - 1; i > 0; i--)
        {
            u16 equalsMask = _mm_movemask_epi8(_mm_cmpeq_epi8(_sse[i], needle));
            if (equalsMask == 0)
                continue;
            
            u16 maxBit = equalsMask & (equalsMask - 1);
            u64 offset = log2(maxBit);

            return iterator(this, (i * _alignment) + offset);
        }

        return end();
    }

    // Modifying Functions

    inline bool Resize(u64 newSize, bool setToZero = false)
    {
        _capacity = Aligned(newSize);
        _buffer = ReallocateBuffer(_buffer, _capacity);

        if (setToZero)
            PlatformZeroMemory(_buffer, _capacity);
        
        return true;
    }

    inline String& Append(const String& other)
    {
        _capacity = Aligned(_length + other._length + 1);
        _buffer = ReallocateBuffer(_buffer, _capacity);
        
        AppendCharsAtOffset(other._sse, other._length, _length);
        _length += other._length;
        return *this;
    }

    inline String& PushBack(char ch)
    {
        if (_length >= _capacity)
        {
            _capacity += _alignment;
            _buffer = ReallocateBuffer(_buffer, _capacity);
            _sse[_length / _alignment] = _mm_setzero_si128();
        }

        _buffer[_length] = ch;
        _length++;

        return *this;
    }

    inline String& PopBack(char ch)
    {
        if (_length > 0)
        {
            _buffer[_length] = '\0';
            _length--;
        }
        
        return *this;
    }

    // Operators
    inline const char& operator[](u64 index) const
    {
        AssertWithMessage(index < _length, "Trying to get to index larger than size!");
        return _buffer[index];
    }

    inline char& operator[](u64 index)
    {
        AssertWithMessage(index < _length, "Trying to get to index larger than size!");
        return _buffer[index];
    }

    inline String operator+(const String& right) const
    {
        String str(_length + right._length + 1);
        str.CopyAlignedBuffer(_sse, Aligned(_length));
        str.AppendCharsAtOffset(right._sse, right._length, _length);
        str._length = _length + right._length;
        return str;
    }

    inline String& operator+=(const String& right)
    {
        if (_capacity < _length + right._length + 1)
        {
            _capacity = Aligned(_length + right._length + 1);
            _buffer = ReallocateBuffer(_buffer, _capacity);
        }
        
        AppendCharsAtOffset(right._sse, right._length, _length);
        _length += right._length;
        return *this;
    }

    inline String& operator=(const char* cstr)
    {
        _length = strlen(cstr);

        if (_capacity <= _length)
        {
            _capacity = Aligned(_length + 1);
            _buffer = ReallocateBuffer(_buffer, _capacity);
        }

        CopyCharBuffer(cstr, _length);
        return *this;
    }

    inline String& operator=(const String& other)
    {
        _length = other._length;

        if (_capacity < other._capacity)
        {
            _capacity = other._capacity;
            _buffer = ReallocateBuffer(_buffer, _capacity);
        }

        CopyAlignedBuffer(other._sse, Aligned(other._length));

        return *this;
    }

    inline String& operator=(String&& other)
    {
        if (_buffer)
            DeallocateBuffer(_buffer, _capacity);

        _length = other._length;
        _capacity = other._capacity;
        _buffer = other._buffer;

        other._length = other._capacity = 0;
        other._buffer = nullptr;

        return *this;
    }

    inline bool operator==(const String& other) const
    {
        if (_length != other._length)
            return false;
        
        u64 batches = Aligned(_length) / _alignment;
        for (u64 i = 0; i < batches; i++)
        {
            u16 equalsMask = _mm_movemask_epi8(_mm_cmpeq_epi8(_sse[i], other._sse[i]));
            if (equalsMask != 0xFFFF)
                return false;
        }

        return true;
    }

    inline bool operator!=(const String& other) const
    {
        if (_length != other._length)
            return true;
        
        u64 batches = Aligned(_length) / _alignment;
        for (u64 i = 0; i < batches; i++)
        {
            u16 equalsMask = _mm_movemask_epi8(_mm_cmpeq_epi8(_sse[i], other._sse[i]));
            if (equalsMask != 0xFFFF)
                return true;
        }

        return false;
    }

    // Pool Functions
    static inline void ResetPool(u64 cap = 16)
    {
        stringPool.Clear();
        stringPool.Resize(cap);
    }

    // Constructors and Destructors
    String()
    :   _buffer(nullptr)
    ,   _length(0)
    ,   _capacity(0)
    {
    }

    String(const char* cstr)
    :   _length(strlen(cstr))
    {
        _capacity = Aligned(_length + 1);
        _buffer = AllocateBufferAsZeros(_capacity);
        CopyCharBuffer(cstr, _length);
    }

    String(u64 size)
    :   _length(0)
    ,   _capacity(Aligned(size))
    {
        _buffer = AllocateBufferAsZeros(_capacity);
    }

    String(const String& other)
    :   _length(other._length)
    ,   _capacity(other._capacity)
    {
        _buffer = AllocateBuffer(_capacity);
        CopyAlignedBuffer(other._sse, Aligned(_length));
    }

    String(String&& other)
    :   _length(other._length)
    ,   _capacity(other._capacity)
    ,   _buffer(other._buffer)
    {
        other._length = other._capacity = 0;
        other._buffer = nullptr;
    }

    ~String()
    {
        if (_buffer)
        {
            DeallocateBuffer(_buffer, _capacity);
            _length = _capacity = 0;
            _buffer = nullptr;
        }
    }

private:
    static constexpr u64 Aligned(u64 num)
    {
        return (num + (_alignment - 1)) & ~(_alignment - 1);
    }

    // Memory functions
    // The strings are pooled. This means that when a string is destoyed,
    // its buffer can be reused for other strings.

    struct PooledString
    {
        char* buffer;
        u64 capacity;

        PooledString(char* buffer, u64 capacity)
        :   buffer(buffer), capacity(capacity)
        {
        }

        ~PooledString()
        {
            if (buffer)
                PlatformFree(buffer);
        }
    };

    static Stack<PooledString, true> stringPool;

    static inline char* AllocateBuffer(u64& bufferSize)
    {
        if (stringPool.size() > 0)
        {
            PooledString s = stringPool.Pop();

            if (bufferSize > s.capacity)
                s.buffer = ReallocateBuffer(s.buffer, bufferSize);
            else
                bufferSize = s.capacity;
            
            char* buffer = s.buffer;
            s.buffer = nullptr;
            return buffer;
        }

        char* buffer = (char*) PlatformAllocate(bufferSize * sizeof(char));
        AssertWithMessage(buffer, "Couldn't allocate string!");
        return buffer;
    }

    static inline char* AllocateBufferAsZeros(u64& bufferSize)
    {
        if (stringPool.size() > 0)
        {
            PooledString s = stringPool.Pop();
            
            if (bufferSize > s.capacity)
                s.buffer = ReallocateBuffer(s.buffer, bufferSize);
            else
                bufferSize = s.capacity;
            
            PlatformZeroMemory(s.buffer, bufferSize * sizeof(char));

            char* buffer = s.buffer;
            s.buffer = nullptr;
            return buffer;
        }

        char* buffer = (char*) PlatformAllocate(bufferSize * sizeof(char));
        AssertWithMessage(buffer, "Couldn't allocate string!");

        PlatformZeroMemory(buffer, bufferSize * sizeof(char));
        return buffer;
    }

    static inline char* ReallocateBuffer(char* buffer, u64 bufferSize)
    {
        char* newBuffer = (char*) PlatformReallocate(buffer, bufferSize * sizeof(char));
        AssertWithMessage(newBuffer, "Couldn't resize string!");
        return newBuffer ? newBuffer : buffer;
    }

    static inline void DeallocateBuffer(char* buffer, u64 capacity)
    {
        // PlatformFree(buffer);
        stringPool.Emplace(buffer, capacity);
    }

    inline void CopyAlignedBuffer(const __m128i* sse, u64 alignedSize)
    {
        // Check alignedSize with capacity
        AssertWithMessage(_capacity >= alignedSize, "Trying to copy elements from a larger string!");

        u64 batches = alignedSize / _alignment;
        for (u64 i = 0; i < batches; i++)
            _mm_store_si128(&_sse[i], sse[i]);
    }

    inline void CopyCharBuffer(const char* buffer, u64 size)
    {
        AssertWithMessage(_capacity >= size + 1, "Trying to copy elements from a larger string!");

        // Copy in batches
        u64 batches = size / _alignment;
        const __m128i* sse = (const __m128i*) buffer;
        for (u64 i = 0; i < batches; i++)
            _mm_store_si128(&_sse[i], sse[i]);

        // Copy remaining
        for (u64 i = batches * _alignment; i < _capacity; i++)
            _buffer[i] = (i < size) ? buffer[i] : '\0';
    }

    inline void AppendCharsAtOffset(const __m128i* sse, u64 size, u64 offset = 0)
    {
        AssertWithMessage(offset < _capacity, "Specified offset lies outside the string!");

        u64 maxAppend = _capacity - offset;
        AssertWithMessage(size < maxAppend, "Trying to append too many chars into string!");

        __m128i* offsetSSE = (__m128i*)(_buffer + offset);
        u64 batches = size / _alignment;
        for (u64 i = 0; i < batches; i++)
            _mm_store_si128(&offsetSSE[i], sse[i]);

        u64 remaining = size % _alignment;
        char* asCharBuffer = (char*) sse;
        for (u64 i = batches * _alignment; (i + offset) < _capacity; i++)
            _buffer[i + offset] = (i < size) ? asCharBuffer[i] : '\0';
    }

private:
    union
    {
        char* _buffer;
        __m128i* _sse;
    };

    u64 _length;
    u64 _capacity;

private:
    friend std::ostream& operator<<(std::ostream& stream, const String& str);
    friend String operator+(const char* left, const String& right);

    friend class StringView;
};

inline String operator+(const char* left, const String& right)
{
    u64 cstrLength = strlen(left);
    String s(cstrLength + right._length + 1);
    s.CopyCharBuffer(left, cstrLength);
    s.AppendCharsAtOffset(right._sse, right._length, cstrLength);
    return s;
}

inline std::ostream& operator<<(std::ostream& stream, const String& str)
{
    stream << str._buffer;
    return stream;
}