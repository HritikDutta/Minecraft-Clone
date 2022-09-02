#include "lexer.h"

#ifdef GN_DEBUG
#include <iostream>
#endif

#include "error_strings.h"
#include "core/types.h"
#include "containers/darray.h"
#include "containers/stringview.h"

namespace json
{

Lexer::Lexer(StringView content)
:   content(content)
{
}

inline static bool IsWhitespace(char ch)
{
    return ch == ' '  ||
           ch == '\t' ||
           ch == '\r' ||
           ch == '\n';
}

inline static bool IsAlphabet(char ch)
{
    return ((ch >= 'a') && (ch <= 'z')) ||
           ((ch >= 'A') && (ch <= 'Z'));
}

inline static bool IsDigit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

inline static void EatSpaces(Lexer& lexer)
{
    while (lexer.currentIndex < lexer.content.size() && IsWhitespace(lexer.content[lexer.currentIndex]))
    {
        lexer.currentLine = (lexer.content[lexer.currentIndex] == '\n');
        lexer.currentIndex++;
    }
}

static inline StringView GetStringToken(Lexer& lexer, StringView& contentView)
{
    // Skip the 1st '"'
    if (contentView[lexer.currentIndex] == '\"')
        lexer.currentIndex++;
    
    u64 start = lexer.currentIndex;
    while (contentView[lexer.currentIndex] != '\"')
    {
        if (contentView[lexer.currentIndex] == '\n' ||
            contentView[lexer.currentIndex] == '\0')
        {
            lexer.errorLineNumber = lexer.currentLine;
            lexer.errorCode = 1;
            break;
        }

        lexer.currentIndex += (contentView[lexer.currentIndex] == '\\');
        lexer.currentIndex++;
    }

    // Skip the 2nd '"'
    lexer.currentIndex++;

    return contentView.SubString(start, lexer.currentIndex - start - 1);
}

static inline StringView GetNumberToken(Lexer& lexer, const StringView& contentView, Token::Type& type)
{
    bool isNegative = (contentView[lexer.currentIndex] == '-');
    bool dotEncountered = false;

    u64 start = lexer.currentIndex;

    lexer.currentIndex += isNegative;

    // Also checking for - in between a number for error checking
    while (IsDigit(contentView[lexer.currentIndex]) ||
           contentView[lexer.currentIndex] == '.'   ||
           contentView[lexer.currentIndex] == '-')
    {
        if (contentView[lexer.currentIndex] == '-')
        {
            lexer.errorLineNumber = lexer.currentLine;
            lexer.errorCode = 2;
            break;
        }

        if (contentView[lexer.currentIndex] == '.')
        {
            if (dotEncountered)
            {
                lexer.errorLineNumber = lexer.currentLine;
                lexer.errorCode = 3;
                break;
            }

            dotEncountered = true;
        }
        lexer.currentIndex++;
    }

    type = (dotEncountered) ? Token::Type::FLOAT : Token::Type::INTEGER;

    return contentView.SubString(start, lexer.currentIndex - start);
}

static inline StringView GetIdentifierToken(Lexer& lexer, const StringView& contentView)
{
    u64 start = lexer.currentIndex;

    while (IsAlphabet(contentView[lexer.currentIndex]))
        lexer.currentIndex++;

    return contentView.SubString(start, lexer.currentIndex - start);
}

#ifdef GN_DEBUG
static void DebugOutput(const Lexer& lexer)
{
    std::cout << "LEXER OUTPUT\n";
    for (auto token : lexer.tokens)
    {
        std::cout << token.value << '\n';
    }
    std::cout << "\n";
}
#endif

void Lexer::Lex()
{
    currentIndex = 0;
    currentLine  = 1;

    errorLineNumber = 0;
    errorCode = 0;

    tokens.Clear();
    tokens.Reserve(std::max((size_t) 2, content.size() / 3)); // Just an estimate

    StringView view = content;

    bool keepLexing = true;
    while (keepLexing && errorCode == 0)
    {
        EatSpaces(*this);

        if (currentIndex >= content.size())
            break;

        switch (content[currentIndex])
        {
            case '\0':
            {
                keepLexing = false;
            } break;

            // Punctuations
            case (char) Token::Type::SQUARE_BRACKET_OPEN:
            case (char) Token::Type::SQUARE_BRACKET_CLOSE:
            case (char) Token::Type::CURLY_BRACKET_OPEN:
            case (char) Token::Type::CURLY_BRACKET_CLOSE:
            case (char) Token::Type::COLON:
            case (char) Token::Type::COMMA:
            {
                auto type = (Token::Type) content[currentIndex];
                tokens.EmplaceBack(type, currentLine, view.SubString(currentIndex, 1));
                currentIndex++;
            } break;

            // Strings
            case '\"':
            {
                tokens.EmplaceBack(Token::Type::STRING, currentLine, GetStringToken(*this, view));
            } break;

            default:
            {
                char startChar = content[currentIndex];
                if (startChar == '-' || startChar == '.' || IsDigit(startChar))
                {
                    Token::Type type;
                    StringView numView = GetNumberToken(*this, view, type);
                    tokens.EmplaceBack(type, currentLine, std::move(numView));
                }
                else
                    tokens.EmplaceBack(Token::Type::IDENTIFIER, currentLine, GetIdentifierToken(*this, view));

            } break;
        }
    }

#   ifdef GN_DEBUG
    // if (errorCode == 0)
    //     DebugOutput(*this);
#   endif
}

const char* Lexer::GetErrorMessage() const
{
    return lexerErrorStrings[errorCode];
}

} // namespace json