#pragma once

#include "core/types.h"
#include "containers/darray.h"
#include "containers/stringview.h"

namespace json
{

struct Token
{
    enum class Type : char
    {
        NONE,
        IDENTIFIER,
        INTEGER,
        FLOAT,
        STRING,

        // Punctuations
        SQUARE_BRACKET_OPEN  = '[',
        SQUARE_BRACKET_CLOSE = ']',
        CURLY_BRACKET_OPEN   = '{',
        CURLY_BRACKET_CLOSE  = '}',
        COLON                = ':',
        COMMA                = ',',

        // Extras
        ILLEGAL,
        NUM_TYPES
    };

    Type type = Type::NONE ;
    u64 lineNumber;
    StringView value;

    Token(Type type, u64 lineNumber, StringView& value)
    :   type(type), lineNumber(lineNumber), value(value)
    {
    }
};

struct Lexer
{
    StringView content;
    DynamicArray<Token> tokens;

    u64 currentIndex;
    u64 currentLine;

    u64 errorLineNumber;
    s32 errorCode;

    Lexer(StringView content);

    void Lex();

    const char* GetErrorMessage() const;
};

} // namespace json