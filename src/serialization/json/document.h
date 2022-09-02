#pragma once

#include "core/types.h"
#include "containers/stringview.h"
#include "containers/darray.h"
#include "containers/hashtable.h"
#include "platform/platform.h"

namespace json
{

using ResourceIndex = u64;
using ArrayNode = DynamicArray<ResourceIndex>;
using ObjectNode = HashTable<String, ResourceIndex>;

struct Resource
{
    enum class Type
    {
        NONE,
        BOOLEAN,
        INTEGER,
        FLOAT,
        STRING
    };

    Type type;

    union
    {
        bool   _boolean;
        s64    _integer;
        f64    _float;
        String _string;
    };

    Resource()
    :   type(Type::NONE)
    {
    }

    Resource(bool _boolean)
    :   type(Type::BOOLEAN)
    ,   _boolean(_boolean)
    {
    }

    Resource(s64 _integer)
    :   type(Type::INTEGER)
    ,   _integer(_integer)
    {
    }

    Resource(f64 _float)
    :   type(Type::FLOAT)
    ,   _float(_float)
    {
    }

    Resource(String&& _string)
    :   type(Type::STRING)
    ,   _string(std::move(_string))
    {
    }

    ~Resource()
    {
        if (type == Type::STRING)
            _string.~String();
    }
};

struct DependencyNode
{
    enum class Type
    {
        DIRECT,
        ARRAY,
        OBJECT
    };

    Type type;

    union
    {
        ResourceIndex _index;
        ArrayNode     _array;
        ObjectNode    _object;
    };

    DependencyNode(Type type)
    :   type(type)
    {
        switch (type)
        {
            case Type::ARRAY:
                _array.ManualInit();
                break;
            
            case Type::OBJECT:
                _object.ManualInit();
                break;
        }
    }

    ~DependencyNode()
    {
        switch (type)
        {
            case Type::ARRAY:
                _array.~DynamicArray();
                break;
            
            case Type::OBJECT:
                _object.~HashTable();
                break;
        }
    }
};

struct Value;

struct Document
{
    DynamicArray<DependencyNode> dependencyTree;
    DynamicArray<Resource> resources;

    Value Start() const;
};


struct Array
{
    const Document& _document;
    size_t _treeIndex;

    Array(const Document& document, size_t index)
    :   _document(document), _treeIndex(index)
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::ARRAY, "Value is not an array!");
    }

    Value operator[](size_t index) const;

    size_t size() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        return node._array.size();
    }

    struct iterator
    {
        const Array* _array;
        ArrayNode::iterator _arrayIt;

        iterator(const Array* _array, ArrayNode::iterator _arrayIt)
        :   _array(_array), _arrayIt(_arrayIt) {}

        iterator& operator++(int)
        {
            _arrayIt++;
            return *this;
        }
        
        iterator operator++()
        {
            iterator it = *this;
            _arrayIt++;
            return it;
        }

        Value operator*() const;

        bool operator==(const iterator& other) const
        {
            return _array == other._array &&
                   _arrayIt == other._arrayIt;
        }

        bool operator!=(const iterator& other) const
        {
            return _array != other._array ||
                   _arrayIt != other._arrayIt;
        }
    };

    iterator begin() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        return iterator(this, node._array.begin());
    }

    iterator end() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        return iterator(this, node._array.end());
    }
};

struct Object
{
    const Document& _document;
    size_t _treeIndex;

    Object(const Document& document, size_t index)
    :   _document(document), _treeIndex(index)
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::OBJECT, "Value is not an object!");
    }

    // Returns null if key isn't found
    Value operator[](const String& key) const;
};

struct Value
{
    const Document& _document;
    size_t _treeIndex;

    Value(const Document& document, size_t index)
    :   _document(document), _treeIndex(index) {}

    const int64_t int64() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::DIRECT, "Value is not an integer!");

        auto& resource = _document.resources[node._index];
        AssertWithMessage(resource.type == Resource::Type::INTEGER, "Value is not an integer!");

        return resource._integer;
    }

    const f64 float64() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::DIRECT, "Value is not a float!");

        auto& resource = _document.resources[node._index];
        AssertWithMessage(resource.type == Resource::Type::FLOAT || resource.type == Resource::Type::INTEGER, "Value is not a float!");

        if (resource.type == Resource::Type::FLOAT)
            return resource._float;
        else
            return (f64) resource._integer;
    }

    const bool boolean() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::DIRECT, "Value is not a bool!");

        auto& resource = _document.resources[node._index];
        AssertWithMessage(resource.type == Resource::Type::BOOLEAN, "Value is not a bool!");

        return resource._boolean;
    }

    const String& string() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::DIRECT, "Value is not a string!");

        auto& resource = _document.resources[node._index];
        AssertWithMessage(resource.type == Resource::Type::STRING, "Value is not a string!");

        return resource._string;
    }

    Array array() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::ARRAY, "Value is not an array!");

        return Array(_document, _treeIndex);
    }

    Value operator[](size_t index) const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::ARRAY, "Value is not an array!");

        return Value(_document, node._array[index]);
    }

    Object object() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::OBJECT, "Value is not an object!");

        return Object(_document, _treeIndex);
    }
    
    // Returns null if key isn't found
    Value operator[](const String& key) const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        AssertWithMessage(node.type == DependencyNode::Type::OBJECT, "Value is not an object!");

        auto val = node._object.Find(key);

        // Return null value if key is not found
        if (val == node._object.end())
            return Value(_document, 0);

        return Value(_document, (*val).value);
    }

    bool IsNull() const
    {
        auto& node = _document.dependencyTree[_treeIndex];
        if (node.type != DependencyNode::Type::DIRECT)
            return false;

        auto& resource = _document.resources[node._index];
        return resource.type == Resource::Type::NONE;
    }
};

} // namespace json