#pragma once

// Can't make a function with a simple type
template <typename Type>
class Function
{
    Function() = delete;
};

template <typename RetType, typename... Args>
class Function <RetType (Args...)>
{
    using FuncType = RetType (*)(Args...);

public:
    RetType operator()(Args... args)
    {
        return _function(args...);
    }

    Function& operator=(const Function& other)
    {
        _function = other._function;
        return *this;
    }

    Function& operator=(Function&& other)
    {
        _function = other._function;
        return *this;
    }

    // Conversion operator
    operator bool() const
    {
        return _function != nullptr;
    }

    Function(FuncType function)
    :   _function(function)
    {
    }

    Function(const Function& other)
    :   _function(other._function)
    {
    }
    
    Function(Function&& other)
    :   _function(other._function)
    {
    }

private:
    FuncType _function;
};