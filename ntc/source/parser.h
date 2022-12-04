#ifndef PARSER_H
#define PARSER_H

#include "list.h"
#include <scanner.h>
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
} NT_NODE;

typedef struct _NT_PARSER
{
    NT_SCANNER *scanner;
    bool hadError;
    NT_TOKEN current;
    NT_TOKEN previous;
} NT_PARSER;

NT_PARSER *ntParserCreate(NT_SCANNER *scanner);
void ntParserDestroy(NT_PARSER *parser);

NT_NODE **ntParse(NT_PARSER *parser, uint32_t *pCount);

void ntPrintNode(uint32_t depth, NT_NODE *node);

NT_NODE *ntRoot(NT_PARSER *parser, NT_LIST types, const bool returnValue);
void ntDestroyNode(NT_NODE *node);

#endif
