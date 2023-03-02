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
#ifndef PARSER_H
#define PARSER_H

#include "list.h"
#include "report.h"
#include "scanner.h"
#include <netuno/type.h>
#include <stdbool.h>

#define strinfy(a, b) a##b

typedef enum _NT_NODE_CLASS
{
#define class(a) NC_##a,
#include "class.inc"
#undef class
    NC_LAST,
} NT_NODE_CLASS;

typedef enum _NT_NODE_KIND
{
#define kind(a) NK_##a,
#include "kind.inc"
#undef kind
    NK_LAST,
} NT_NODE_KIND;

typedef enum _NT_LITERAL_TYPE
{
#define literal(a) LT_##a,
#include "literal.inc"
#undef literal
    LT_LAST,
} NT_LITERAL_TYPE;

typedef struct _NT_NODE_TYPE
{
    NT_NODE_CLASS class;
    NT_NODE_KIND kind;
    NT_LITERAL_TYPE literalType;
} NT_NODE_TYPE;

typedef struct _NT_NODE NT_NODE;
typedef struct _NT_NODE
{
    NT_NODE_TYPE type;
    NT_TOKEN token;
    NT_TOKEN token2;

    NT_NODE *left;
    NT_NODE *right;
    NT_NODE *condition;

    NT_LIST data;

    const NT_TYPE *expressionType;
    void *userdata;
} NT_NODE;

typedef struct _NT_PARSER
{
    NT_SCANNER *scanner;
    NT_TOKEN current;
    NT_TOKEN previous;
    NT_REPORT report;
} NT_PARSER;

NT_PARSER *ntParserCreate(NT_SCANNER *scanner);
void ntParserDestroy(NT_PARSER *parser);

NT_NODE *ntParse(NT_PARSER *parser);

const char *ntGetKindLabel(NT_NODE_KIND kind);
const char *ntGetClassLabel(NT_NODE_CLASS class);
const char *ntGetLiteralTypeLabel(NT_LITERAL_TYPE type);
void ntPrintNode(uint32_t depth, NT_NODE *node);

NT_NODE *ntRoot(NT_PARSER *parser, NT_LIST types, const bool returnValue);
void ntDestroyNode(NT_NODE *node);

#endif
