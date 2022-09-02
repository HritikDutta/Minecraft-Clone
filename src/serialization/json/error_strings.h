#pragma once

constexpr char lexerErrorStrings[][64] = {
    "No Error Encountered",
    "String was never closed!",
    "- sign can only be used at the start of the number!",
    ". can only be used once in a number!",
};

constexpr char parserErrorStrings[][64] = {
    "No Error Encountered",
    "An identifier can only be true, false or null",
    "Elements in an array must be separated by commas",
    "Key value pairs in an object must be separated by commas",
    "Key string not found in object!",
    "Keys and values must be separated by a colon!",
    "Value expected!",
    "Value not found!",
    "Object was never closed with a }",
    "Array was never closed with a ]",
    "End of file expected!",
    "Unexpected escape character!",
};