#include "codegen.h"
#include "parser.h"
#include "path.h"
#include "resolver.h"
#include "scanner.h"
#include <assert.h>
#include <ctype.h>
#include <netuno/console.h>
#include <netuno/memory.h>
#include <netuno/ntc.h>
#include <netuno/str.h>
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

static bool insertModuleSymbol(NT_SYMBOL_TABLE *table, const NT_MODULE *module)
{
    assert(table);
    assert(module);
    assert(IS_VALID_OBJECT(module));

    const NT_SYMBOL_ENTRY entry = (NT_SYMBOL_ENTRY){
        .symbol_name = module->type.typeName,
        .target_label = NULL,
        .type = SYMBOL_TYPE_MODULE | SYMBOL_TYPE_PUBLIC,
        .data = (void *)module,
        .data2 = 0,
        .exprType = &module->type,
        .weak = false,
    };

    return ntInsertSymbol(table, &entry);
}

NT_ASSEMBLY *ntCompile(NT_ASSEMBLY *assembly, size_t fileCount, const NT_FILE *files)
{
    assert(assembly != NULL);
    assert(files != NULL);
    assert(fileCount > 0);

    NT_NODE **nodes = ntMalloc(sizeof(NT_NODE *) * fileCount);
    NT_SYMBOL_TABLE *globalTable = ntCreateSymbolTable(NULL, STT_NONE, NULL);

    insertModuleSymbol(globalTable, ntConsoleModule());

    for (size_t i = 0; i < fileCount; ++i)
    {
        const NT_FILE *const current = &files[i];

        char_t *filename = ntPathFilename(current->source, false);

        NT_SCANNER *scanner = ntScannerCreate(current->code, filename);
        NT_PARSER *parser = ntParserCreate(scanner);
        nodes[i] = ntParse(parser);

        ntParserDestroy(parser);
        ntScannerDestroy(scanner);

        assert(nodes[i]->userdata);
        assert(IS_VALID_OBJECT(nodes[i]->userdata));
        assert(((NT_OBJECT *)nodes[i]->userdata)->type->objectType == NT_OBJECT_TYPE_TYPE);
        assert(((NT_TYPE *)nodes[i]->userdata)->objectType == NT_OBJECT_MODULE);

        insertModuleSymbol(globalTable, (NT_MODULE *)nodes[i]->userdata);
        ntFree(filename);
    }

    const bool resolveResult = ntResolve(assembly, globalTable, fileCount, nodes);
    assert(resolveResult);
    if (!resolveResult)
    {
        assembly = NULL;
        goto error;
    }

    NT_CODEGEN *codegen = ntCreateCodegen(assembly);
    const bool genResult = ntGen(codegen, fileCount, (const NT_NODE **)nodes);
    assert(genResult);
    if (!genResult)
    {
        assembly = NULL;
        goto error;
    }

    ntFreeCodegen(codegen);

error:
    for (size_t i = 0; i < fileCount; ++i)
    {
        ntDestroyNode(nodes[i]);
    }
    ntFree(nodes);

    return assembly;
}
