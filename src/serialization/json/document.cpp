#include "document.h"

namespace json
{

Value Document::Start() const
{
    return Value(*this, 1);
}

Value Array::operator[](size_t index) const
{
    auto& node = _document.dependencyTree[_treeIndex];
    return Value(_document, node._array[index]);
}

Value Array::iterator::operator*() const
{
    return Value(_array->_document, *_arrayIt);
}

Value Object::operator[](const String& key) const
{
    auto& node = _document.dependencyTree[_treeIndex];
    auto val = node._object.Find(key);

    // Return null value if key is not found
    if (val == node._object.end())
        return Value(_document, 0);

    return Value(_document, (*val).value);
}

} // namespace json