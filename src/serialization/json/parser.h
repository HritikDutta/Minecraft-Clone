#pragma once

#include "containers/stringview.h"
#include "document.h"
#include "lexer.h"

namespace json
{

struct Parser
{
    u64 currentTokenIndex;
    void ParseLexedOutput(const Lexer& lexer, Document& out);

    s32 errorCode;
    s32 errorLineNumber;

    const char* GetErrorMessage() const;
};

bool ParseJsonString(StringView json, Document& document);

} // namespace json
