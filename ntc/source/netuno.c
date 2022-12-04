#include <codegen.h>
#include <ctype.h>
#include <netuno/memory.h>
#include <netuno/ntc.h>
#include <parser.h>
#include <scanner.h>
#include <stdio.h>
#include <string.h>

char_t *toCharT(const char *str)
{
    const size_t len = strlen(str);
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char *i = str; *i != 0; i += mbrtoc32(NULL, i, len + str - i, &ps))
        size++;

    char_t *s = (char_t *)ntMalloc((size + 1) * sizeof(char_t));
    char_t *j = s;

    for (const char *i = str; *i != 0; i += mbrtoc32(j++, i, len + str - i, &ps))
        ;

    s[size] = 0;
    return s;
}

size_t strlenT(const char_t *str)
{
    size_t s = 0;
    for (const char_t *i = str; *i != 0; ++i)
        s++;
    return s;
}

char *toChar(const char_t *str)
{
    const size_t len = strlenT(str) + 1;
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char_t *i = str; *i != 0; i++)
        size += c32rtomb(NULL, *i, &ps);

    char *s = (char *)ntMalloc((size + 1) * sizeof(char));

    char *j = s;
    for (const char_t *i = str; *i != 0; i++)
        j += c32rtomb(j, *i, &ps);
    return s;
}

char *toCharS(const char_t *str, size_t len)
{
    len += 1;
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps));

    size_t size = 0;
    for (const char_t *i = str; (size_t)(i - str) < len; i++)
        size += c32rtomb(NULL, *i, &ps);

    char *s = (char *)ntMalloc((size + 1) * sizeof(char));

    char *j = s;
    for (const char_t *i = str; (size_t)(i - str) < len; i++)
        j += c32rtomb(j, *i, &ps);

    s[size] = 0;
    return s;
}

static const NT_KEYWORD_PAIR KEYWORDS[] = {
#define op(id, str) {str, id},
#define keyword(id, str) {str, id},
#include <keywords.inc>
#undef op
#undef keyword
};

NT_CHUNK *ntCompile(NT_ASSEMBLY *assembly, const char_t *str)
{
    // char_t *s = toCharT(str);
    NT_SCANNER *scanner = ntScannerCreate(str);
    NT_PARSER *parser = ntParserCreate(scanner);

    uint32_t count;
    NT_NODE **root = ntParse(parser, &count);
    for (uint32_t i = 0; i < count; i++)
        ntPrintNode(0, root[i]);

    NT_CHUNK *chunk = ntCreateChunk();
    NT_CODEGEN *codegen = ntCreateCodegen(assembly, chunk);
    ntGen(codegen, (const NT_NODE **)root, count);
    for (uint32_t i = 0; i < count; i++)
        ntDestroyNode(root[i]);
    ntFree(root);

    ntScannerDestroy(scanner);
    ntParserDestroy(parser);
    ntFreeCodegen(codegen);

    // free(s);

    ntArrayAdd(&assembly->chunks, &chunk, sizeof(NT_CHUNK *));

    return chunk;
}

void testNetuno(void)
{
    char sample[] = "def main(x, y) {\n"
                    "    print(1)"
                    "}\n";
    // char sample[] = "def pow(x, y) {\n"
    //                 "    var tmp = 1\n"
    //                 "    var p = 0\n"
    //                 "    while p < y {\n"
    //                 "        tmp = tmp * x\n"
    //                 "        p = p + 1\n"
    //                 "    }\n"
    //                 "    return tmp\n"
    //                 "}\n";

    char_t *s = toCharT(sample);
    char *ds = toChar(s);
    printf("%s\n", ds);

    NT_SCANNER *scanner = ntScannerCreate(s);

    NT_TOKEN token;
    NT_PARSER *parser = ntParserCreate(scanner);

    uint32_t count;
    NT_NODE **root = ntParse(parser, &count);
    for (uint32_t i = 0; i < count; i++)
        ntPrintNode(0, root[i]);
    return;

    // if (token.type != TK_EOF)
    // {
    //     do
    //     {
    //         if (token.type != TK_KEYWORD)
    //         {
    //             char *lexeme = toCharS(token.lexeme, token.lexemeLength);
    //             printf("Token: %x, Lexeme: %s\n", token.type, lexeme);
    //             free(lexeme);
    //         }
    //         else
    //         {
    //             const char_t *str = NULL;
    //             bool find = false;

    //             for (uint32_t i = 0; i < sizeof(KEYWORDS) / sizeof(NT_KEYWORD_PAIR); ++i)
    //             {
    //                 if (KEYWORDS[i].value == token.id)
    //                 {
    //                     find = true;
    //                     str = KEYWORDS[i].keyword;
    //                 }
    //             }
    //             if (find)
    //             {
    //                 char *lexeme = toCharS(str, strlenT(str));
    //                 printf("Token: %x, Keyword: %s\n", token.type, lexeme);
    //                 free(lexeme);
    //             }
    //             else
    //                 printf("Token: %x, Keyword: %c\n", token.type, token.id);
    //         }
    //         ntScanToken(scanner, &token);
    //     } while (token.type != TK_EOF);
}
