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
#include "console.h"
#include "helper.h"

static NT_TYPE CONSOLE = {
    .objectType = NT_TYPE_MODULE,
    .typeName = NULL,
    .baseType = NULL,
};

static void addWrite(NIR_CONTEXT *context, NT_TYPE *type, NIR_MODULE *module)
{
    NT_PARAM param = {
        .type = ntObjectType(),
        .name = ntCopyString(U"object", 6),
    };

    const NT_TYPE *delegateType = ntTakeDelegateType(ntVoidType(), 1, &param);
    addFunction(context, type, U"write", SYMBOL_TYPE_SUBROUTINE, delegateType, module);
}

static void addReadline(NIR_CONTEXT *context, NT_TYPE *type, NIR_MODULE *module)
{
    const NT_TYPE *delegateType = ntTakeDelegateType(ntStringType(context), 0, NULL);
    addFunction(context, type, U"readline", SYMBOL_TYPE_FUNCTION, delegateType, module);
}

const NT_TYPE *ntConsoleModule(NIR_CONTEXT *context)
{
    if (CONSOLE.typeName == NULL)
    {
        const char_t *moduleName = U"console";
        CONSOLE.typeName = ntCopyString(moduleName, ntStrLen(moduleName));
        CONSOLE.baseType = ntObjectType();

        NIR_MODULE *module = nirCreateModule(moduleName);

        ntInitSymbolTable(&CONSOLE.fields, &CONSOLE.baseType->fields, STT_TYPE, NULL);

        CONSOLE.fields.scopeReturnType = &CONSOLE;
        CONSOLE.fields.data = module;

        addWrite(context, &CONSOLE, module);
        addReadline(context, &CONSOLE, module);
    }

    return &CONSOLE;
}
