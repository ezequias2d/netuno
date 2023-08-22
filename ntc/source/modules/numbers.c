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
#include "numbers.h"
#include "helper.h"
#include "netuno/memory.h"
#include "netuno/nil/basic_block.h"
#include "netuno/str.h"
#include "netuno/string.h"
#include "scope.h"
#include "string.h"
#include "type.h"

static NT_TYPE STRING = {
    .objectType = NT_TYPE_STRING,
    .typeName = NULL,
};

static void addCast(NIL_CONTEXT *context, NT_TYPE *type, const NT_TYPE *sourceCast,
                    const NT_TYPE *targetCast, NIL_MODULE *module)
{
    NT_PARAM param = {
        .type = sourceCast,
        .name = ntCopyString(U"object", 6),
    };

    const NT_TYPE *delegateType = ntTakeDelegateType(targetCast, 1, &param);

    const char_t *prefix = U"to_";
    const size_t prefixLength = ntStrLen(prefix);
    const size_t sourceNameLength = ntStringLength(targetCast->typeName);
    const char_t *sourceName = ntStringChars(targetCast->typeName, NULL);

    char_t *name = (char_t *)ntMalloc(sizeof(char_t) * (prefixLength + sourceNameLength + 1));
    ntMemcpy(name, prefix, prefixLength * sizeof(char_t));
    ntMemcpy(name + prefixLength, sourceName, sourceNameLength * sizeof(char_t));
    name[prefixLength + sourceNameLength] = U'\0';

    addFunction(context, type, name, SYMBOL_TYPE_FUNCTION, delegateType, module);

    ntFree(name);
}

// static void addEquals(NIL_CONTEXT *context, NT_TYPE *type, NIL_MODULE *module)
// {
//     NT_PARAM params[2] = {
//         {
//             .type = ntStringType(context),
//             .name = ntCopyString(U"left", 4),
//         },
//         {
//             .type = ntStringType(context),
//             .name = ntCopyString(U"right", 5),
//         },
//     };

//     const NT_TYPE *delegateType = ntTakeDelegateType(ntBoolType(context), 2, params);
//     addFunction(context, type, U"equals", SYMBOL_TYPE_FUNCTION, delegateType, module);
// }

static void startPrimitive(NT_TYPE *type, NIL_CONTEXT *context)
{
    NIL_MODULE *module = nilCreateModule(ntStringChars(type->typeName, NULL));

    STRING.fields.scopeReturnType = &STRING;
    STRING.fields.data = module;

    addCast(context, type, type, ntStringType(context), module);
    addCast(context, type, ntStringType(context), type, module);
}

static NT_TYPE I32_TYPE = {
    .objectType = NT_TYPE_I32,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntI32Type(NIL_CONTEXT *context)
{
    if (I32_TYPE.typeName == NULL)
    {
        I32_TYPE.typeName = ntCopyString(U"int", 3);
        ntInitSymbolTable(&I32_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&I32_TYPE, context);
    }
    return &I32_TYPE;
}

static NT_TYPE BOOL_TYPE = {
    .objectType = NT_TYPE_BOOL,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntBoolType(NIL_CONTEXT *context)
{
    if (BOOL_TYPE.typeName == NULL)
    {
        BOOL_TYPE.typeName = ntCopyString(U"bool", 4);
        ntInitSymbolTable(&BOOL_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&BOOL_TYPE, context);
    }
    return &BOOL_TYPE;
}

static NT_TYPE I64_TYPE = {
    .objectType = NT_TYPE_I64,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntI64Type(NIL_CONTEXT *context)
{
    if (I64_TYPE.typeName == NULL)
    {
        I64_TYPE.typeName = ntCopyString(U"long", 4);
        ntInitSymbolTable(&I64_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&I64_TYPE, context);
    }
    return &I64_TYPE;
}

static NT_TYPE U32_TYPE = {
    .objectType = NT_TYPE_U32,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntU32Type(NIL_CONTEXT *context)
{
    if (U32_TYPE.typeName == NULL)
    {
        U32_TYPE.typeName = ntCopyString(U"uint", 4);
        ntInitSymbolTable(&U32_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&U32_TYPE, context);
    }
    return &U32_TYPE;
}

static NT_TYPE U64_TYPE = {
    .objectType = NT_TYPE_U64,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntU64Type(NIL_CONTEXT *context)
{
    if (U64_TYPE.typeName == NULL)
    {
        U64_TYPE.typeName = ntCopyString(U"ulong", 5);
        ntInitSymbolTable(&U64_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&U64_TYPE, context);
    }
    return &U64_TYPE;
}

static NT_TYPE F32_TYPE = {
    .objectType = NT_TYPE_F32,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntF32Type(NIL_CONTEXT *context)
{
    if (F32_TYPE.typeName == NULL)
    {
        F32_TYPE.typeName = ntCopyString(U"float", 5);
        ntInitSymbolTable(&F32_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&F32_TYPE, context);
    }
    return &F32_TYPE;
}

static NT_TYPE F64_TYPE = {
    .objectType = NT_TYPE_F64,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntF64Type(NIL_CONTEXT *context)
{
    if (F64_TYPE.typeName == NULL)
    {
        F64_TYPE.typeName = ntCopyString(U"double", 6);
        ntInitSymbolTable(&F64_TYPE.fields, NULL, STT_TYPE, 0);

        startPrimitive(&F64_TYPE, context);
    }
    return &F64_TYPE;
}
