#ifndef TOKEN_H
#define TOKEN_H

#include <netuno/ntc.h>
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
    TK_LABEL,

    TK_EOF,
    TK_ERROR,
} NT_TK_TYPE;

typedef enum _NT_TK_ID
{
    TK_ID_NONE,
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
    uint32_t current;
    int32_t line;
} NT_SCANNER;

NT_SCANNER *ntScannerCreate(const char_t *source);
void ntScannerDestroy(NT_SCANNER *scanner);
const char_t *ntGetKeywordLexeme(const NT_TK_ID id);

void ntScanToken(NT_SCANNER *scanner, NT_TOKEN *result);
bool ntIsAtEnd(const NT_SCANNER *scanner);
#endif
