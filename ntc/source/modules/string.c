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
#include "string.h"
#include "helper.h"
#include "netuno/memory.h"
#include "netuno/str.h"
#include "netuno/string.h"
#include "numbers.h"
#include "scope.h"
#include "type.h"

static NT_TYPE STRING = {
    .objectType = NT_TYPE_STRING,
    .typeName = NULL,
};

static void addCast(NIL_CONTEXT *context, NT_TYPE *type, const NT_TYPE *sourceCast,
                    NIL_MODULE *module)
{
    NT_PARAM param = {
        .type = sourceCast,
        .name = ntCopyString(U"object", 6),
    };

    const NT_TYPE *delegateType = ntTakeDelegateType(&STRING, 1, &param);

    const char_t *prefix = U"to_string_";
    const size_t prefixLength = ntStrLen(prefix);
    const size_t sourceNameLength = ntStringLength(sourceCast->typeName);
    const char_t *sourceName = ntStringChars(sourceCast->typeName, NULL);

    char_t *name = (char_t *)ntMalloc(sizeof(char_t) * (prefixLength + sourceNameLength + 1));
    ntMemcpy(name, prefix, prefixLength * sizeof(char_t));
    ntMemcpy(name + prefixLength, sourceName, sourceNameLength * sizeof(char_t));
    name[prefixLength + sourceNameLength] = U'\0';

    addFunction(context, type, name, SYMBOL_TYPE_FUNCTION, delegateType, module);

    ntFree(name);
}

static void addEquals(NIL_CONTEXT *context, NT_TYPE *type, NIL_MODULE *module)
{
    NT_PARAM params[2] = {
        {
            .type = ntStringType(context),
            .name = ntCopyString(U"left", 4),
        },
        {
            .type = ntStringType(context),
            .name = ntCopyString(U"right", 5),
        },
    };

    const NT_TYPE *delegateType = ntTakeDelegateType(ntBoolType(context), 2, params);
    addFunction(context, type, U"equals", SYMBOL_TYPE_FUNCTION, delegateType, module);
}

static void addConcat(NIL_CONTEXT *context, NT_TYPE *type, NIL_MODULE *module)
{
    NT_PARAM params[2] = {
        {
            .type = ntStringType(context),
            .name = ntCopyString(U"left", 4),
        },
        {
            .type = ntStringType(context),
            .name = ntCopyString(U"right", 5),
        },
    };

    const NT_TYPE *delegateType = ntTakeDelegateType(ntStringType(context), 2, params);
    addFunction(context, type, U"concat", SYMBOL_TYPE_FUNCTION, delegateType, module);
}

const NT_TYPE *ntStringType(NIL_CONTEXT *context)
{
    if (context && STRING.typeName == NULL)
    {
        const char_t *moduleName = U"string";
        STRING.typeName = ntCopyString(moduleName, ntStrLen(moduleName));
        STRING.baseType = ntObjectType();
        ntInitSymbolTable(&STRING.fields, &(ntObjectType()->fields), STT_TYPE, 0);

        NIL_MODULE *module = nilCreateModule(moduleName);

        STRING.fields.scopeReturnType = &STRING;
        STRING.fields.data = module;

        addCast(context, &STRING, ntBoolType(context), module);
        addCast(context, &STRING, ntI32Type(context), module);
        addCast(context, &STRING, ntI64Type(context), module);
        addCast(context, &STRING, ntU32Type(context), module);
        addCast(context, &STRING, ntU64Type(context), module);
        addCast(context, &STRING, ntF32Type(context), module);
        addCast(context, &STRING, ntF64Type(context), module);

        addEquals(context, &STRING, module);
        addConcat(context, &STRING, module);
    }
    return &STRING;
}
