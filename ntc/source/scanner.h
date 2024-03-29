/*
MIT License

Copyright (c) 2022 Ezequias Silva <ezequiasmoises@gmail.com> and the Netuno
contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef TOKEN_H
#define TOKEN_H

#include <trie.h>

typedef struct _NT_TRIE NT_TRIE;

typedef enum _NT_TK_TYPE
{
    TK_NONE,
    TK_CHAR,
    TK_KEYWORD,
    TK_I32,
    TK_I64,
    TK_U32,
    TK_U64,
    TK_F32,
    TK_F64,
    TK_IDENT,
    TK_STRING,

    TK_EOF,
    TK_ERROR,
} NT_TK_TYPE;

typedef enum _NT_TK_ID
{
    TK_ID_NONE,
    TK_GT = '>',
    TK_LT = '<',
    TK_PLUS = '+',
    TK_MINUS = '-',
    TK_MULT = '*',
    TK_DIV = '/',
    TK_MOD = '%',
    TK_OR = '|',
    TK_AND = '&',
    TK_XOR = '^',
    TK_BANG = '!',
    TK_BITWISE_NOT = '~',
    TK_ = 255,
#define op(name, _) name,
#define keyword(name, x) name,
#include "keywords.inc"
#undef op
#undef keyword
} NT_TK_ID;

typedef struct _NT_TOKEN
{
    NT_TK_TYPE type;
    int32_t line;
    const char_t *pLine;
    const char_t *sourceName;

    // TK_KEYWORD
    NT_TK_ID id;
    // Others
    struct
    {
        const char_t *lexeme;
        uint32_t lexemeLength;
    };
} NT_TOKEN;

typedef struct _NT_SCANNER
{
    NT_TRIE *keywords;
    const char_t *source;
    const char_t *sourceName;
    uint32_t current;
    int32_t line;
    const char_t *pLine;
} NT_SCANNER;

NT_SCANNER *ntScannerCreate(const char_t *source, const char_t *sourceName);
void ntScannerDestroy(NT_SCANNER *scanner);
const char_t *ntGetKeywordLexeme(const NT_TK_ID id);

void ntScanToken(NT_SCANNER *scanner, NT_TOKEN *result);
bool ntIsAtEnd(const NT_SCANNER *scanner);
#endif
