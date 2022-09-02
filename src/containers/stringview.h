#pragma once

/*

View into a String.
Basically a cstring with a size;

*/

#include <cstring>
#include "string.h"
#include "core/logging.h"

class StringView
{
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

        inline const char& operator*() const
        {
            AssertWithMessage(_index <= _view->_length, "Trying to dereference a non existant value!");
            return _view->_bufferPtr[_index];
        }

        inline bool operator==(const iterator& other) const
        {
            return _view == other._view &&
                   _index == other._index;
        }

        inline bool operator!=(const iterator& other) const
        {
            return _view != other._view ||
                   _index != other._index;
        }

        // Conversions
        inline operator bool() const
        {
            return *this != _view->end();
        }

        // Getters
        inline u64 index() const
        {
            return _index;
        }

        // Constructors
        iterator(const StringView* view, u64 index)
        :   _view(view), _index(index)
        {
        }
    
    private:
        inline void advance()
        {
            if (_index > _view->_length)
                return;
            
            _index++;
        }

    private:
        const StringView* _view;
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

    inline const char* cstr() const { return _bufferPtr; };

    // Search Functions
    inline iterator FindFirstOf(char ch) const
    {
        __m128i needle = _mm_set1_epi8(ch);
        __m128i* asSSE = (__m128i*) _bufferPtr;

        u64 batches = _length / String::_alignment;
        for (int i = 0; i < batches; i++)
        {
            u16 equalsMask = _mm_movemask_epi8(_mm_cmpeq_epi8(asSSE[i], needle));
            if (equalsMask == 0)
                continue;
            
            u16 minBit = equalsMask & ~(equalsMask - 1);
            u64 offset = log2(minBit);

            return iterator(this, (i * String::_alignment) + offset);
        }

        // Normal search for whatever doesn't fit in 16 bytes
        for (int i = batches * String::_alignment; i < _length; i++)
        {
            if (_bufferPtr[i] == ch)
                return iterator(this, (batches * String::_alignment) + i);
        }

        return end();
    }

    inline iterator FindLastOf(char ch) const
    {
        __m128i needle = _mm_set1_epi8(ch);
        __m128i* asSSE = (__m128i*) _bufferPtr;

        u64 batches = _length / String::_alignment;

        // Just do the reverse of FindFirstOf

        for (int i = batches * String::_alignment; i < _length; i++)
        {
            if (_bufferPtr[i] == ch)
                return iterator(this, (batches * String::_alignment) + i);
        }

        for (int i = batches - 1; i >= 0; i--)
        {
            u16 equalsMask = _mm_movemask_epi8(_mm_cmpeq_epi8(asSSE[i], needle));
            if (equalsMask == 0)
                continue;
            
            u16 maxBit = equalsMask & (equalsMask - 1);
            u64 offset = log2(maxBit);

            return iterator(this, (i * String::_alignment) + offset);
        }

        return end();
    }

    // SubString
    inline StringView SubString(u64 start, u64 count = 18446744073709551615Ui64) const
    {
        return StringView(_bufferPtr + start, count);
    }

    // Operators
    inline const char& operator[](u64 index) const
    {
        AssertWithMessage(index < _length, "Trying to get to index larger than size!");
        return _bufferPtr[index];
    }

    inline StringView& operator=(const StringView& other)
    {
        _bufferPtr = other._bufferPtr;
        _length = other._length;

        return *this;
    }

    inline bool operator==(const StringView& other) const
    {
        if (_length != other._length)
            return false;
        
        // TODO: Optimize for SIMD
        for (u64 i = 0; i < _length; i++)
        {
            if (_bufferPtr[i] != other._bufferPtr[i])
                return false;
        }

        return true;
    }

    
    inline bool operator!=(const StringView& other) const
    {
        if (_length != other._length)
            return true;
        
        // TODO: Optimize for SIMD
        for (u64 i = 0; i < _length; i++)
        {
            if (_bufferPtr[i] != other._bufferPtr[i])
                return true;
        }

        return false;
    }
    
    // Converting to Strings
    inline operator String() const
    {
        String s(_length + 1);
        s.CopyCharBuffer(_bufferPtr, _length);
        s._length = _length;
        return std::move(s);
    }

    // Constructors and Destructors
    StringView(const char* cstr)
    :   _bufferPtr(cstr)
    ,   _length(strlen(cstr))
    {
    }

    StringView(const String& str)
    :   _bufferPtr(str.cstr())
    ,   _length(str._length)
    {
    }

    StringView(const String& str, u64 start, u64 count = 18446744073709551615Ui64)
    :   _bufferPtr(str.cstr() + start)
    ,   _length((str._length - start <= count) ? str._length - start : count)
    {
    }

    StringView(const StringView& sv)
    :   _bufferPtr(sv._bufferPtr)
    ,   _length(sv._length)
    {
    }

    StringView(StringView&& sv)
    :   _bufferPtr(sv._bufferPtr)
    ,   _length(sv._length)
    {
        sv._bufferPtr = nullptr;
        sv._length = 0;
    }

private:
    StringView(const char* _bufferPtr, u64 _length)
    :   _bufferPtr(_bufferPtr)
    ,   _length(_length)
    {
    }

private:
    const char* _bufferPtr;
    u64 _length;
};

// Output
inline std::ostream& operator<<(std::ostream& stream, const StringView& sv)
{
    for (const char ch : sv)
        stream << ch;
    
    return stream;
}