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

NT_ASSEMBLY *ntCompile(NT_ASSEMBLY *assembly, const char_t *str, const char_t *sourceName)
{
    NT_SCANNER *scanner = ntScannerCreate(str, sourceName);
    NT_PARSER *parser = ntParserCreate(scanner);

    uint32_t count;
    NT_NODE **root = ntParse(parser, &count);

#ifndef NDEBUG
    for (uint32_t i = 0; i < count; i++)
        ntPrintNode(0, root[i]);
#endif

    NT_CODEGEN *codegen = ntCreateCodegen(assembly);
    ntGen(codegen, (const NT_NODE **)root, count);
    for (uint32_t i = 0; i < count; i++)
        ntDestroyNode(root[i]);
    ntFree(root);

    ntScannerDestroy(scanner);
    ntParserDestroy(parser);
    ntFreeCodegen(codegen);

    return assembly;
}
