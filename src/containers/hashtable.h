#pragma once

#include <cstdint>
#include <cstdlib>
#include "hash.h"
#include "core/logging.h"

template <typename Key, typename Value, typename Hasher = Hasher<Key>>
class HashTable
{
private:
    static constexpr u64 START_CAP = 16;
    static constexpr f32 GROWTH_RATE = 2.0f;
    static constexpr f32 MAX_LOAD_FACTOR = 0.8f;

    enum struct State
    {
        EMPTY,
        TOMBSTONE,
        FILLED
    };

    struct TableData
    {
        State* states { nullptr };
        Hash*  hashes { nullptr };
        Key*   keys   { nullptr };
        Value* values { nullptr };

        inline TableData& operator=(const TableData& other)
        {
            states = other.states;
            hashes = other.hashes;
            keys   = other.keys;
            values = other.values;

            return *this;
        }
    };

public:
    // Data structure for dereferencing iterators
    struct KeyValuePair
    {
        Key& key;
        Value& value;

        KeyValuePair(Key& key, Value& value)
        :   key(key), value(value)
        {
        }
    };

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

        inline KeyValuePair operator*()
        {
            AssertWithMessage(_index <= _table->_last, "Trying to dereference a non existant value!");
            return KeyValuePair(_table->_table.keys[_index], _table->_table.values[_index]);
        }

        inline const KeyValuePair operator*() const
        {
            AssertWithMessage(_index <= _table->_last, "Trying to dereference a non existant value!");
            return KeyValuePair(_table->_table.keys[_index], _table->_table.values[_index]);
        }

        inline bool operator==(const iterator& other) const
        {
            return _table == other._table &&
                   _index == other._index;
        }

        inline bool operator!=(const iterator& other) const
        {
            return _table != other._table ||
                   _index != other._index;
        }

        // Conversions
        inline operator bool() const
        {
            return *this != _table->end();
        }
        
        // Getters
        inline u64 index() const
        {
            return _index;
        }

        inline Key& key() const
        {
            return _table->_table.keys[_index];
        }

        inline Value& value() const
        {
            return _table->_table.values[_index];
        }

        // Constructors
        iterator(const HashTable* table, u64 index)
        :   _table(table), _index(index)
        {
        }

        iterator(const iterator& other)
        :   _table(other._table), _index(other._index)
        {
        }

    private:
        inline void advance()
        {
            if (_index > _table->_last)
                return;
            
            _index++;
            while (_table->_table.states[_index] != State::FILLED &&
                   _index <= _table->_last)
                _index++;
        }

    private:
        const HashTable* _table;
        u64 _index;
    };

    inline const iterator begin() const { return iterator(this, _first); }
    inline       iterator begin()       { return iterator(this, _first); }

    inline const iterator end() const { return iterator(this, _last + 1); }
    inline       iterator end()       { return iterator(this, _last + 1); }

public:
    // Getters
    inline u64 size()     const { return _size; }
    inline u64 capacity() const { return _capacity; }

    // Operators
    inline const Value& operator[](const Key& key) const
    {
        return Find(key).value();
    }
    
    inline Value& operator[](const Key& key)
    {
        return At(key).value();
    }

    // Explicit Functions
    inline void ManualInit(u64 capacity = START_CAP)
    {
        Allocate(_table, capacity);
        
        _capacity = capacity;
        _size = 0;

        _first = capacity;
        _last = 0;
    }

    inline void Rehash(u64 capacity)
    {
        TableData newTable;
        Allocate(newTable, capacity);

        u64 newFirst = capacity;
        u64 newLast = 0;

        for (u64 i = _first; i <= _last; i++)
        {
            if (_table.states[i] != State::FILLED)
                continue;
            
            const Hash& hash = _table.hashes[i];
            u64 startIndex = hash % capacity;

            for (u64 j = startIndex + 1; j != startIndex; j = (j + 1) % capacity)
            {
                if (newTable.states[j] == State::EMPTY)
                {
                    newTable.states[j] = State::FILLED;
                    newTable.hashes[j] = hash;
                    newTable.keys[j] = std::move(_table.keys[i]);
                    newTable.values[j] = std::move(_table.values[i]);

                    newFirst = std::min(newFirst, j);
                    newLast = std::max(newLast, j);

                    break;
                }
            }
        }

        _capacity = capacity;

        _first = newFirst;
        _last = newLast;

        Deallocate(_table);
        _table = newTable;
    }

    // Tries to find the element
    // If not found, returns end()
    inline iterator Find(const Key& key) const
    {
        u64 hash = hasher(key);
        u64 startIndex = hash % _capacity;

        for (u64 i = (startIndex + 1) % _capacity; i != startIndex; i = (i + 1) % _capacity)
        {
            const State& currentPairState = _table.states[i];

            if (currentPairState == State::EMPTY)
                return end();
            
            if (currentPairState == State::TOMBSTONE)
                continue;
            
            if (_table.hashes[i] == hash &&
                _table.keys[i] == key)
                return iterator(this, i);
        }

        // Shouldn't reach this point
        return end();
    }

    // Tries to find the element
    // If not found, places an empty element and returns that
    inline iterator At(const Key& key)
    {
        if (GetLoadFactor() >= MAX_LOAD_FACTOR)
            Rehash(_capacity * GROWTH_RATE);

        u64 hash = hasher(key);
        u64 startIndex = hash % _capacity;

        for (u64 i = (startIndex + 1) % _capacity; i != startIndex; i = (i + 1) % _capacity)
        {
            const State& currentPairState = _table.states[i];

            if (currentPairState != State::FILLED)
            {
                CopyKey(i, key);

                _table.states[i] = State::FILLED;
                _table.hashes[i] = hash;
                _table.values[i] = Value();     // Represents empty value

                _first = std::min(_first, i);
                _last  = std::max(_last, i);

                _size++;

                return iterator(this, i);
            }
            
            if (_table.hashes[i] == hash &&
                _table.keys[i] == key)
                return iterator(this, i);
        }

        // Shouldn't reach this point
        return end();
    }

    inline Value& Place(const Key& key, const Value& value)
    {
        if (GetLoadFactor() >= MAX_LOAD_FACTOR)
            Rehash(_capacity * GROWTH_RATE);

        Hash hash = hasher(key);
        u64 startIndex = hash % _capacity;

        for (u64 i = (startIndex + 1) % _capacity; i != startIndex; i = (i + 1) % _capacity)
        {
            State& currentPairState = _table.states[i];
            if (currentPairState !=  State::FILLED)
            {
                CopyKey(i, key);

                currentPairState = State::FILLED;
                _table.hashes[i] = hash;
                _table.values[i] = value;

                _first = std::min(_first, i);
                _last  = std::max(_last, i);

                _size++;
                return _table.values[i];
            }

            if (_table.hashes[i] == hash &&
                _table.key[i] == key)
            {
                if (_table.values[i] != value)
                    _table.values[i] = value;
                
                return _table.values[i];
            }
        }

        // Shouldn't reach this point
        return _table.values[_capacity];
    }

    inline Value& Place(const Key& key, Value&& value)
    {
        if (GetLoadFactor() >= MAX_LOAD_FACTOR)
            Rehash(_capacity * GROWTH_RATE);

        Hash hash = hasher(elem);
        u64 startIndex = hash % _capacity;

        for (u64 i = (startIndex + 1) % _capacity; i != startIndex; i = (i + 1) % _capacity)
        {
            State& currentPairState = _table.states[i];
            if (currentPairState != State::FILLED)
            {
                CopyKey(i, key);

                currentPairState = State::FILLED;
                _table.hashes[i] = hash;
                _table.values[i] = std::move(value);

                _first = std::min(_first, i);
                _last = std::max(_last, i);

                _size++;
                return _table.values[i];
            }

            if (_table.hashes[i] == hash &&
                _table.keys[i] == key)
            {
                _table.values[i] = std::move(value);

                return _table.values[i];
            }
        }

        // Shouldn't reach this point
        return _table.values[_capacity];
    }

    template <typename... Args>
    inline Value& Emplace(const Key& key, Args&&... args)
    {
        if (GetLoadFactor() >= MAX_LOAD_FACTOR)
            Rehash(_capacity * GROWTH_RATE);

        T elem(std::forward<Args>(args)...);

        Hash hash = hasher(elem);
        u64 startIndex = hash % _capacity;

        for (u64 i = (startIndex + 1) % _capacity; i != startIndex; i = (i + 1) % _capacity)
        {
            State& currentPairState = _table.states[i];
            if (currentPairState != State::FILLED)
            {
                CopyKey(key);

                currentPairState = State::FILLED;
                _table.hashes[i] = hash;
                new(&_table.values + i) Value(std::forward<Args>(args)...);

                _first = std::min(_first, i);
                _last = std::max(_last, i);

                _size++;
                return _table.elements[i];
            }

            if (_table.hashes[i]   == hash &&
                _table.elements[i] == elem)
            {
                _table.values[i] = Value(std::forward<Args>(args)...);
                return _table.values[i];
            }
        }

        // Shouldn't reach this point
        return _table.values[_capacity];
    }

    inline void Remove(const Key& key)
    {
        iterator it = Find(key);

        if (it == end())
        {
            Warn("Trying to remove non-existant key in table.");
            return;
        }

        u64 index = it.index();

        _table.states[index] = State::TOMBSTONE;
        _table.keys[index].~Key();
        _table.values[index].~Value();
        _size--;

        // Update First and Last
        if (_size == 0)
        {
            _first = _capacity;
            _last = 0;
        }
        else if (index == _first)
        {
            for (u64 i = _first + 1; i <= _last; i++)
            {
                if (_table.states[i] == State::FILLED)
                {
                    _first = i;
                    break;
                }
            }
        }
        else if (index == _last)
        {
            for (u64 i = _last - 1; i >= _first; i--)
            {
                if (_table.states[i] == State::FILLED)
                {
                    _last = i;
                    break;
                }
            }
        }
    }

    // Constructors and Destructors
    HashTable(u64 capacity = START_CAP)
    :   _size(0), _capacity(capacity)
    ,   _first(capacity), _last(0)
    {
        Allocate(_table, capacity);
    }

    ~HashTable()
    {
        for (u64 i = _first; i < _last; i++)
        {
            if (_table.states[i] == State::FILLED)
            {
                _table.keys[i].~Key();
                _table.values[i].~Value();
            }
        }

        Deallocate(_table);
    }

private:
    inline float GetLoadFactor() const
    {
        return (float) _size / (float) _capacity;
    }

    inline void CopyKey(u64 index, const Key& key)
    {
        if (_table.states[index] == State::EMPTY)
            new (_table.keys + index) Key(key);
        else
            _table.keys[index] = key;
    }

    static inline void Allocate(TableData& table, u64 elements)
    {
        State* ptr = (State*) PlatformAllocate(elements * (sizeof(Key) + sizeof(Value) + sizeof(Hash) + sizeof(State)));
        AssertWithMessage(ptr, "Couldn't allocate table.");

        table.states = ptr;
        table.hashes = (Hash*)(table.states + elements);
        table.keys   = (Key*)(table.hashes + elements);
        table.values = (Value*)(table.keys + elements);

        PlatformSetMemory(table.states, (int) State::EMPTY, elements * sizeof(State));
    }

    static inline void Deallocate(TableData& table)
    {
        if (table.states)
        {
            PlatformFree(table.states);

            table.states = nullptr;
            table.hashes = nullptr;
            table.keys   = nullptr;
            table.values = nullptr;
        }
    }

private:
    TableData _table;
    u64 _first, _last;
    u64 _size, _capacity;

    Hasher hasher;
};