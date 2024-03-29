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
#include "codegen.h"
#include "parser.h"
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

        NT_SCANNER *scanner = ntScannerCreate(current->code, current->filename);
        NT_PARSER *parser = ntParserCreate(scanner);
        nodes[i] = ntParse(parser);

        ntParserDestroy(parser);
        ntScannerDestroy(scanner);

        NT_MODULE *const module = (NT_MODULE *)nodes[i]->userdata;
        assert(module);
        assert(IS_VALID_OBJECT(module));
        assert(IS_TYPE(module, ntModuleType()));

        insertModuleSymbol(globalTable, module);
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
