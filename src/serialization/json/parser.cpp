#include "parser.h"

#ifdef GN_DEBUG
#include <iostream>
#endif

#include "error_strings.h"
#include "document.h"
#include "lexer.h"

namespace json
{

#ifdef GN_DEBUG

static u64 PrintNodeInfo(const Document& document, ResourceIndex nodeIndex)
{
    auto& node = document.dependencyTree[nodeIndex];

    u64 offset = 1;

    switch (node.type)
    {
        case DependencyNode::Type::DIRECT:
        {
            auto& resource = document.resources[node._index];
            switch (resource.type)
            {
                case Resource::Type::STRING:
                std::cout << resource._string << '\n';
                break;

                case Resource::Type::INTEGER:
                std::cout << resource._integer << '\n';
                break;

                case Resource::Type::FLOAT:
                std::cout << resource._float << '\n';
                break;
                
                case Resource::Type::BOOLEAN:
                std::cout << resource._boolean << '\n';
                break;

                case Resource::Type::NONE:
                std::cout << "null\n";
                break;
            }
        } break;

        case DependencyNode::Type::ARRAY:
        {
            std::cout << "::Array Start::\n";
            
            for (size_t index : node._array)
                offset += PrintNodeInfo(document, index);

            std::cout << "::Array End::\n";
        } break;

        case DependencyNode::Type::OBJECT:
        {
            std::cout << "::Object Start::\n";
            
            for (auto& pair : node._object)
            {
                std::cout << pair.key << ":\n";
                offset += PrintNodeInfo(document, pair.value);
            }

            std::cout << "::Object End::\n";
        } break;
    }

    return offset;
}

static void DebugOutput(const Document& document)
{
    std::cout << "PARSER OUTPUT" << std::endl;

    for (size_t index = 1; index < document.dependencyTree.size(); index++)
    {
        index += PrintNodeInfo(document, index);
    }
}

#endif

static String EscapeToken(Parser& parser, const Token& token)
{
    String escaped(token.value.size());

    auto& view = token.value;

    for (size_t i = 0; i < view.size() && parser.errorCode == 0; i++)
    {
        if (view[i] == '\\')
        {
            i++;
            switch (view[i])
            {
                case 'b' : escaped.PushBack('\b'); break;
                case 'f' : escaped.PushBack('\f'); break;
                case 'n' : escaped.PushBack('\n'); break;
                case 'r' : escaped.PushBack('\r'); break;
                case 't' : escaped.PushBack('\t'); break;
                case '\"': escaped.PushBack('\"'); break;
                case '\\': escaped.PushBack('\\'); break;
                default:
                {
                    parser.errorCode = 11;
                    parser.errorLineNumber = token.lineNumber;
                } break;
            }

            continue;
        }
        
        escaped.PushBack(view[i]);
    }

    return std::move(escaped);
}

static void ParseNext(Parser& parser, const Lexer& lexer, Document& out);

static void ParseArray(Parser& parser, const Lexer& lexer, Document& out)
{
    size_t myIndex = out.dependencyTree.size();
    out.dependencyTree.EmplaceBack(DependencyNode::Type::ARRAY);

    // Skip the first [
    parser.currentTokenIndex++;

    while (parser.errorCode == 0)
    {
        if (parser.currentTokenIndex >= lexer.tokens.size())
        {
            parser.errorCode = 9;
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex - 1].lineNumber;
            break;
        }

        if (lexer.tokens[parser.currentTokenIndex].type == Token::Type::SQUARE_BRACKET_CLOSE)
            break;
        
        auto& node = out.dependencyTree[myIndex];
        node._array.EmplaceBack(out.dependencyTree.size());
        ParseNext(parser, lexer, out);

        if (parser.errorCode != 0)
            break;

        if (parser.currentTokenIndex >= lexer.tokens.size())
        {
            parser.errorCode = 8;
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex - 1].lineNumber;
            break;
        }

        if (lexer.tokens[parser.currentTokenIndex].type == Token::Type::SQUARE_BRACKET_CLOSE)
            break;

        if (lexer.tokens[parser.currentTokenIndex].type != Token::Type::COMMA)
        {
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex].lineNumber;
            parser.errorCode = 2;
            break;
        }

        parser.currentTokenIndex++;
    }
}

static void ParseObject(Parser& parser, const Lexer& lexer, Document& out)
{
    size_t myIndex = out.dependencyTree.size();
    out.dependencyTree.EmplaceBack(DependencyNode::Type::OBJECT);

    // Skip the first {
    parser.currentTokenIndex++;
    
    while (parser.errorCode == 0)
    {
        if (parser.currentTokenIndex >= lexer.tokens.size())
        {
            parser.errorCode = 8;
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex - 1].lineNumber;
            break;
        }

        if (lexer.tokens[parser.currentTokenIndex].type == Token::Type::CURLY_BRACKET_CLOSE)
            break;

        auto& node = out.dependencyTree[myIndex];

        auto& keyToken = lexer.tokens[parser.currentTokenIndex++];
        if (keyToken.type != Token::Type::STRING)
        {
            parser.errorLineNumber = keyToken.lineNumber;
            parser.errorCode = 4;
            break;
        }

        {   // Check for semi colon
            auto& token = lexer.tokens[parser.currentTokenIndex++];
            if (token.type != Token::Type::COLON)
            {
                parser.errorLineNumber = token.lineNumber;
                parser.errorCode = 5;
                break;
            }
        }

        String keyString = EscapeToken(parser, keyToken);
        if (parser.errorCode != 0)
            break;

        node._object[keyString] = out.dependencyTree.size(); 
        ParseNext(parser, lexer, out);

        if (parser.errorCode != 0)
            break;

        if (parser.currentTokenIndex >= lexer.tokens.size())
        {
            parser.errorCode = 8;
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex - 1].lineNumber;
            break;
        }

        if (lexer.tokens[parser.currentTokenIndex].type == Token::Type::CURLY_BRACKET_CLOSE)
            break;

        if (lexer.tokens[parser.currentTokenIndex].type != Token::Type::COMMA)
        {
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex].lineNumber;
            parser.errorCode = 3;
            break;
        }

        parser.currentTokenIndex++;
    }
}

static void ParseNext(Parser& parser, const Lexer& lexer, Document& out)
{
    if (parser.currentTokenIndex >= lexer.tokens.size())
    {
        parser.errorCode = 6;
        parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex - 1].lineNumber;
        return;
    }

    auto& token = lexer.tokens[parser.currentTokenIndex];

    switch (token.type)
    {
        case Token::Type::STRING:
        {
            size_t resourceIndex = out.resources.size();

            {   // Push Resource
                String value = EscapeToken(parser, token);
                out.resources.EmplaceBack(std::move(value));
            }

            {   // Push Node
                auto& node = out.dependencyTree.EmplaceBack(DependencyNode::Type::DIRECT);
                node._index = resourceIndex;
            }
        } break;

        case Token::Type::INTEGER:
        {
            size_t resourceIndex = out.resources.size();

            {   // Push Resource
                String numString = token.value;
                int64_t value = _atoi64(numString.cstr());
                out.resources.EmplaceBack(value);
            }

            {   // Push Node
                auto& node = out.dependencyTree.EmplaceBack(DependencyNode::Type::DIRECT);
                node._index = resourceIndex;
            }
        } break;

        case Token::Type::FLOAT:
        {
            size_t resourceIndex = out.resources.size();

            {   // Push Resource
                String numString = token.value;
                double value = atof(numString.cstr());
                out.resources.EmplaceBack(value);
            }

            {   // Push Node
                auto& node = out.dependencyTree.EmplaceBack(DependencyNode::Type::DIRECT);
                node._index = resourceIndex;
            }
        } break;

        case Token::Type::IDENTIFIER:
        {
            size_t resourceIndex = out.resources.size();
            
            {   // Push Resource
                if (token.value == "true")
                    out.resources.EmplaceBack(true);
                else if (token.value == "false")
                    out.resources.EmplaceBack(false);
                else if (token.value == "null")
                    resourceIndex = 0;    // Set to the common null resource
                else
                {
                    parser.errorLineNumber = token.lineNumber;
                    parser.errorCode = 1;
                    break;
                }
            }
            
            {   // Push Node
                auto& node = out.dependencyTree.EmplaceBack(DependencyNode::Type::DIRECT);
                node._index = resourceIndex;
            }
        } break;

        // Array
        case Token::Type::SQUARE_BRACKET_OPEN:
        {
            ParseArray(parser, lexer, out);
        } break;

        // Array
        case Token::Type::CURLY_BRACKET_OPEN:
        {
            ParseObject(parser, lexer, out);
        } break;

        default:
        {
            parser.errorCode = 7;
            parser.errorLineNumber = lexer.tokens[parser.currentTokenIndex].lineNumber;
        } break;
    }

    parser.currentTokenIndex++;
}

void Parser::ParseLexedOutput(const Lexer& lexer, Document& out)
{
    currentTokenIndex = 0;
    errorCode = 0;

    out.dependencyTree.Clear();
    out.resources.Clear();

    // This is a null element
    // If user tries to access an object property that wasn't in the file,
    // then the value will point to this element
    auto& node = out.dependencyTree.EmplaceBack(DependencyNode::Type::DIRECT);
    node._index = 0;
    out.resources.EmplaceBack();

    if (lexer.tokens.size() > 0)
    {
        ParseNext(*this, lexer, out);

        // Check if more tokens are remaining after parsing
        if (errorCode == 0 && currentTokenIndex < lexer.tokens.size())
        {
            errorLineNumber = lexer.tokens[currentTokenIndex].lineNumber;
            errorCode = 10;
        }
    }

#   ifdef GN_DEBUG
    // if (errorCode == 0)
    //     DebugOutput(out);
#   endif
}

const char* Parser::GetErrorMessage() const
{
    return parserErrorStrings[errorCode];
}

bool ParseJsonString(StringView json, Document& document)
{
    json::Lexer lexer(json);
    lexer.Lex();

    if (lexer.errorCode != 0)
        return false;

    json::Parser parser;
    parser.ParseLexedOutput(lexer, document);

    if (parser.errorCode != 0)
        return false;

    return true;
}

} // namespace json
