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
#include "modules/console.h"
#include "nil_codegen.h"
#include "parser.h"
#include "report.h"
#include "resolver.h"
#include "scanner.h"
#include "scope.h"
#include <assert.h>
#include <ctype.h>
#include <netuno/nil/context.h>
// #include <netuno/console.h>
#include <netuno/memory.h>
#include <netuno/ntc.h>
#include <netuno/str.h>
#include <stdio.h>
#include <string.h>

static bool insertModuleSymbol(NT_SCOPE *table, const NT_TYPE *module)
{
    assert(table);
    assert(module);

    const NT_SYMBOL entry = (NT_SYMBOL){
        .symbol_name = module->typeName,
        .target_label = NULL,
        .type = SYMBOL_TYPE_MODULE | SYMBOL_TYPE_PUBLIC,
        .exprType = module,
        .weak = false,
    };

    return ntInsertSymbol(table, &entry);
}

bool ntCompile(size_t fileCount, const NT_FILE *files, NIL_MODULE **modules)
{
    assert(files != NULL);
    assert(fileCount > 0);

    NT_NODE **nodes = ntMalloc(sizeof(NT_NODE *) * fileCount);
    NT_SCOPE *globalScope = ntCreateSymbolTable(NULL, STT_NONE, NULL);

    NIL_CONTEXT *context = nilCreateContext();
    insertModuleSymbol(globalScope, ntConsoleModule(context));

    for (size_t i = 0; i < fileCount; ++i)
    {
        const NT_FILE *const current = &files[i];

        NT_SCANNER *scanner = ntScannerCreate(current->code, current->filename);
        NT_PARSER *parser = ntParserCreate(scanner);
        nodes[i] = ntParse(parser);

        ntParserDestroy(parser);
        ntScannerDestroy(scanner);
    }

    const bool resolveResult = ntResolve(context, globalScope, fileCount, nodes);
    if (!resolveResult)
        goto error;

    bool had_error = false;
    for (size_t i = 0; i < fileCount; ++i)
    {
        modules[i] = ntNirGen(context, nodes[i]);
        if (modules[i] == NULL)
            had_error = true;
    }

    if (had_error)
    {
        for (size_t i = 0; i < fileCount; ++i)
        {
            if (modules[i])
                nilDestroyModule(modules[i]);
        }
        goto error;
    }

    return true;
error:
    for (size_t i = 0; i < fileCount; ++i)
    {
        ntDestroyNode(nodes[i]);
    }
    ntFree(nodes);
    return false;
}
