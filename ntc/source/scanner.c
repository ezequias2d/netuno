#include "scanner.h"
#include <ctype.h>
#include <netuno/memory.h>
#include <netuno/str.h>

static const NT_KEYWORD_PAIR KEYWORDS[] = {
#define op(id, str) {str, id},
#define keyword(id, str) {str, id},
#include <keywords.inc>
#undef op
#undef keyword
};

static const uint32_t KEYWORDS_COUNT = sizeof(KEYWORDS) / sizeof(NT_KEYWORD_PAIR);

NT_SCANNER *ntScannerCreate(const char_t *source)
{
    NT_SCANNER *scanner = (NT_SCANNER *)ntMalloc(sizeof(NT_SCANNER));

    *scanner = (NT_SCANNER){.keywords = ntTrieCreate(KEYWORDS_COUNT, KEYWORDS),
                            .source = source,
                            .current = 0,
                            .line = 0};

    return scanner;
}

const char_t *ntGetKeywordLexeme(const NT_TK_ID id)
{
    for (size_t i = 0; i < KEYWORDS_COUNT; ++i)
    {
        if (KEYWORDS[i].value == id)
        {
            return KEYWORDS[i].keyword;
        }
    }
    return U"";
}

void ntScannerDestroy(NT_SCANNER *scanner)
{
    if (scanner != NULL)
    {
        ntTrieDestroy(scanner->keywords);
        ntFree(scanner);
    }
}

static char_t peek(const NT_SCANNER *scanner)
{
    return scanner->source[scanner->current];
}

static char_t peekNext(const NT_SCANNER *scanner)
{
    if (ntIsAtEnd(scanner))
        return '\0';

    return scanner->source[scanner->current + 1];
}

bool ntIsAtEnd(const NT_SCANNER *scanner)
{
    return peek(scanner) == '\0';
}

static char_t advance(NT_SCANNER *scanner)
{
    if (ntIsAtEnd(scanner))
        return '\0';
    return scanner->source[scanner->current++];
}

static bool match(NT_SCANNER *scanner, char_t v)
{
    if (ntIsAtEnd(scanner) || scanner->source[scanner->current] != v)
        return false;
    scanner->current++;
    return true;
}

static void skipWhitespaces(NT_SCANNER *scanner)
{
    while (true)
    {
        const char_t c = peek(scanner);
        switch (c)
        {
        case '\n':
            scanner->line++;
            advance(scanner);
            break;
        case ' ':
        case '\r':
        case '\t':
            advance(scanner);
            break;
        case ';':
            while (peek(scanner) != '\n' && !ntIsAtEnd(scanner))
                advance(scanner);
        default:
            return;
        }
    }
}

static void errorToken(const NT_SCANNER *scanner, const char_t *message, NT_TOKEN *result)
{
    *result = (NT_TOKEN){.type = TK_ERROR,
                         .lexeme = message,
                         .lexemeLength = ntStrLen(message),
                         .line = scanner->line};
}

static void makeToken(const NT_SCANNER *scanner, NT_TK_TYPE type, NT_TOKEN *result)
{
    *result = (NT_TOKEN){
        .type = type,
        .lexeme = scanner->source,
        .lexemeLength = scanner->current,
        .line = scanner->line,
    };
}

static void makeKeyword(const NT_SCANNER *scanner, NT_TK_ID id, NT_TOKEN *result)
{
    *result = (NT_TOKEN){.type = TK_KEYWORD,
                         .lexeme = scanner->source,
                         .lexemeLength = scanner->current,
                         .id = id,
                         .line = scanner->line};
}

static bool isAlpha(char_t c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isDigit(char_t c)
{
    return c >= '0' && c <= '9';
}

static bool isXDigit(char_t c)
{
    return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool isOctDigit(char_t c)
{
    return c >= '0' && c <= '7';
}

static void identifier(NT_SCANNER *scanner, NT_TOKEN *result)
{
    char_t c = peek(scanner);
    while (isAlpha(c) || isDigit(c))
    {
        advance(scanner);
        c = peek(scanner);
    }

    uint32_t mnemoic = TK_ERROR;
    if (ntTrieTryGet(scanner->keywords, scanner->source, scanner->current, &mnemoic))
    {
        makeKeyword(scanner, mnemoic, result);
        return;
    }

    NT_TK_TYPE tokenType = TK_IDENT;
    if (c == ':')
    {
        advance(scanner);
        tokenType = TK_LABEL;
    }
    makeToken(scanner, tokenType, result);
}

static void number(NT_SCANNER *scanner, NT_TOKEN *result)
{
    while (isDigit(peek(scanner)))
        advance(scanner);

    NT_TK_TYPE type = TK_I32;
    // look for fractional part
    if (peek(scanner) == '.' && isDigit(peekNext(scanner)))
    {
        type = TK_F64;

        // consume '.'
        advance(scanner);
        while (isDigit(peek(scanner)))
            advance(scanner);

        if (peek(scanner) == 'f')
            type = TK_F32;
    }
    else
    {
        switch (peek(scanner))
        {
        case 'i':
            type = TK_I32;
            break;
        case 'u':
            if (peekNext(scanner) == 'l')
                type = TK_U64;
            else
                type = TK_U32;
            break;
        case 'l':
            type = TK_I64;
            break;
        }
    }

    makeToken(scanner, type, result);
}

static void string(NT_SCANNER *scanner, NT_TOKEN *result)
{
    while (peek(scanner) != '"' && !ntIsAtEnd(scanner))
    {
        if (peek(scanner) == '\n')
            scanner->line++;
        advance(scanner);
    }

    if (ntIsAtEnd(scanner))
    {
        errorToken(scanner, U"Unterminated string.", result);
        return;
    }

    // the closing quote
    advance(scanner);
    makeToken(scanner, TK_STRING, result);
}

static char_t hex_char(NT_SCANNER *scanner)
{
    char_t c = peek(scanner);
    char_t r = 0;
    while (isXDigit(c))
    {
        if (c >= '0' && c <= U'9')
            r = (r << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f')
            r = (r << 4) | (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            r = (r << 4) | (c - 'A' + 10);
        advance(scanner);
        c = peek(scanner);
    }
    return r;
}

static char_t oct_char(NT_SCANNER *scanner)
{
    char_t c = peek(scanner);
    char_t r = 0;
    while (isOctDigit(c))
    {
        r = (r << 3) | (c - '0');
        advance(scanner);
        c = peek(scanner);
    }
    return r;
}

// static bool isValidUnicode(const char_t c)
// {
//     return c == (c & 0x1FFFFF);
// }

static char_t unicode_char(NT_SCANNER *scanner, const uint32_t len)
{
    char_t r = 0;
    for (uint32_t i = 0; i < len; ++i)
    {
        const char_t c = advance(scanner);

        if (c >= '0' && c <= '9')
            r = (r << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f')
            r = (r << 4) | (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            r = (r << 4) | (c - 'A' + 10);
        else
            return 0x200000;
    }
    return r;
}

static char_t escapeChar(NT_SCANNER *scanner)
{
    const char_t c = peek(scanner);
    switch (c)
    {
    case '\'':
    case '"':
    case '?':
    case '\\':
        return c;
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case 'e':
        return '\x1B';
    case 'x':
        return hex_char(scanner);
    case 'u':
        return unicode_char(scanner, 4);
    case 'U':
        return unicode_char(scanner, 8);
    case '0':
        return oct_char(scanner);
    default:
        return '\0';
    }
}

static void character(NT_SCANNER *scanner, NT_TOKEN *result)
{
    char_t c = advance(scanner);
    // escape
    if (c == '\\')
    {
        c = escapeChar(scanner);
        if (c == '\0')
        {
            errorToken(scanner, U"Invalid scape sequence!", result);
            return;
        }
    }

    if (!match(scanner, '\''))
        errorToken(scanner, U"Unterminated char.", result);
}

void ntScanToken(NT_SCANNER *scanner, NT_TOKEN *result)
{
    skipWhitespaces(scanner);

    scanner->source += scanner->current;
    scanner->current = 0;

    if (ntIsAtEnd(scanner))
    {
        makeToken(scanner, TK_EOF, result);
        return;
    }

    const char_t c = advance(scanner);

    if (isAlpha(c))
    {
        identifier(scanner, result);
        return;
    }

    if (isDigit(c))
    {
        number(scanner, result);
        return;
    }

    switch (c)
    {
    case '+':
        makeKeyword(scanner, match(scanner, '+') ? OP_INC : (match(scanner, '=') ? OP_A_ADD : '+'),
                    result);
        break;
    case '-':
        makeKeyword(scanner, match(scanner, '-') ? OP_DEC : (match(scanner, '=') ? OP_A_SUB : '-'),
                    result);
        break;
    case '*':
        makeKeyword(scanner, match(scanner, '=') ? OP_A_MUL : '*', result);
        break;
    case '=':
        makeKeyword(scanner, match(scanner, '=') ? OP_EQ : '=', result);
        break;
    case '!':
        makeKeyword(scanner, match(scanner, '=') ? OP_NE : '!', result);
        break;
    case '&':
        makeKeyword(scanner, match(scanner, '=') ? OP_LOGAND : '&', result);
        break;
    case '|':
        makeKeyword(scanner, match(scanner, '=') ? OP_LOGOR : '|', result);
        break;
    case '^':
        makeKeyword(scanner, match(scanner, '=') ? OP_A_XOR : '^', result);
        break;
    case '/':
        makeKeyword(scanner, match(scanner, '=') ? OP_A_DIV : '/', result);
        break;
    case '<':
        makeKeyword(scanner,
                    match(scanner, '<') ? (match(scanner, '=') ? OP_A_SAL : OP_SAL)
                                        : (match(scanner, '=') ? OP_LE : '<'),
                    result);
        break;
    case '>':
        makeKeyword(scanner,
                    match(scanner, '>') ? (match(scanner, '=') ? OP_A_SAR : OP_SAR)
                                        : (match(scanner, '=') ? OP_GE : '>'),
                    result);
        break;
    case '%':
        makeToken(scanner, match(scanner, '=') ? OP_A_MOD : '%', result);
        break;
    case '.':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case '?':
    case '~':
    case ',':
    case ':':
        makeKeyword(scanner, c, result);
        break;
    case '\'':
        character(scanner, result);
        break;
    case '"':
        advance(scanner);
        string(scanner, result);
        break;
    default:
        break;
    }
}
