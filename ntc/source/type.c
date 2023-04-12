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
#include "type.h"
#include "scope.h"
#include <assert.h>
#include <netuno/array.h>
#include <netuno/common.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <stdio.h>
#include <string.h>

bool ntTypeIsAssignableFrom(const NT_TYPE *to, const NT_TYPE *from)
{
    assert(to);
    assert(from);

    const NT_TYPE *previous = NULL;
    do
    {
        if (to == from)
            return true;

        previous = from;
        from = from->baseType;
    } while (from != NULL && previous != from);

    return false;
}

static NT_TYPE OBJECT_TYPE = {
    .objectType = NT_TYPE_OBJECT,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntObjectType(void)
{
    if (OBJECT_TYPE.typeName == NULL)
    {
        OBJECT_TYPE.typeName = ntCopyString(U"object", 6);
        OBJECT_TYPE.baseType = NULL;
        ntInitSymbolTable(&OBJECT_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &OBJECT_TYPE;
}

static NT_TYPE UNDEFINED_TYPE = {
    .objectType = NT_TYPE_UNDEFINED,
    .typeName = NULL,

    .baseType = NULL,
};

const NT_TYPE *ntUndefinedType(void)
{
    if (UNDEFINED_TYPE.typeName == NULL)
    {
        UNDEFINED_TYPE.typeName = ntCopyString(U"undefined", 9);
        ntInitSymbolTable(&UNDEFINED_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &UNDEFINED_TYPE;
}

static NT_TYPE VOID_TYPE = {
    .objectType = NT_TYPE_VOID,
    .typeName = NULL,

    .baseType = NULL,
};

const NT_TYPE *ntVoidType(void)
{
    if (VOID_TYPE.typeName == NULL)
    {
        VOID_TYPE.typeName = ntCopyString(U"void", 4);
        ntInitSymbolTable(&VOID_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &VOID_TYPE;
}

static NT_TYPE ERROR_TYPE = {
    .objectType = NT_TYPE_ERROR,
    .typeName = NULL,
    .baseType = NULL,
};

const NT_TYPE *ntErrorType(void)
{
    if (ERROR_TYPE.typeName == NULL)
    {
        ERROR_TYPE.typeName = ntCopyString(U"error", 5);
        ntInitSymbolTable(&ERROR_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &ERROR_TYPE;
}

static NT_TABLE typeTable = {.count = 0, .pEntries = NULL};

static NT_TYPE *ntCreateType(NT_TYPE_ID objectType, NT_STRING *typeName, const NT_TYPE *baseType)
{
    NT_TYPE *type = (NT_TYPE *)ntMalloc(sizeof(NT_TYPE));
    type->objectType = objectType;
    type->typeName = typeName;
    type->baseType = baseType;

    ntTableSet(&typeTable, ntRefString(typeName), type);

    const NT_SCOPE *parent = NULL;
    if (baseType)
        parent = &baseType->fields;

    ntInitSymbolTable(&type->fields, parent, STT_TYPE, type);

    switch (objectType)
    {
    case NT_TYPE_DELEGATE:
        type->delegate.returnType = ntUndefinedType();
        type->delegate.paramCount = 0;
        type->delegate.params = NULL;
        break;
    default:
        break;
    }

    return type;
}

NT_TYPE *ntTakeType(NT_TYPE_ID objectType, NT_STRING *typeName, const NT_TYPE *baseType)
{
    NT_TYPE *type;
    if (!ntTableGet(&typeTable, typeName, (void **)&type))
    {
        type = ntCreateType(objectType, typeName, baseType);
    }
    return type;
}

static char_t *ntDelegateTypeName(const NT_TYPE *returnType, size_t paramCount,
                                  const NT_PARAM *params)
{
    NT_ARRAY array;
    ntInitArray(&array);

    const char_t *const delegate = U"delegate(";
    ntArrayAdd(&array, delegate, sizeof(char_t) * ntStrLen(delegate));
    for (size_t i_param = 0; i_param < paramCount; ++i_param)
    {
        const NT_PARAM *param = &params[i_param];
        assert(param->type);
        const NT_STRING *const typeName = param->type->typeName;

        size_t length;
        const char_t *chars = ntStringChars(typeName, &length);

        ntArrayAdd(&array, chars, length * sizeof(char_t));
    }
    ntArrayAdd(&array, U")", sizeof(char_t));

    if (returnType)
    {
        size_t length;
        const char_t *chars = ntStringChars(returnType->typeName, &length);
        assert(chars);

        ntArrayAdd(&array, U":", sizeof(char_t));
        ntArrayAdd(&array, chars, sizeof(char_t) * length);
    }

    const char_t term = '\0';
    ntArrayAdd(&array, &term, sizeof(char_t));

    return ntRealloc(array.data, array.count);
}

static NT_TYPE *ntCreateDelegateType(NT_STRING *delegateTypeName, const NT_TYPE *returnType,
                                     size_t paramCount, const NT_PARAM *params)
{
    NT_TYPE *type = ntCreateType(NT_TYPE_DELEGATE, delegateTypeName, ntObjectType());

    type->delegate.paramCount = paramCount;
    type->delegate.returnType = returnType;

    type->delegate.params = (NT_PARAM *)ntMalloc(sizeof(NT_PARAM) * paramCount);
    ntMemcpy(type->delegate.params, params, sizeof(NT_PARAM) * paramCount);

    return type;
}

const NT_TYPE *ntTakeDelegateType(const NT_TYPE *returnType, size_t paramCount,
                                  const NT_PARAM *params)
{
    NT_STRING *delegateName;
    {
        char_t *name = ntDelegateTypeName(returnType, paramCount, params);
        const size_t len = ntStrLen(name);
        delegateName = ntTakeString(name, len);
    }

    NT_TYPE *type;
    if (!ntTableGet(&typeTable, delegateName, (void **)&type))
    {
        type = ntCreateDelegateType(delegateName, returnType, paramCount, params);
    }
    return type;
}
