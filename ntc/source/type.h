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
#ifndef NT_TYPE_H
#define NT_TYPE_H

#include "scope.h"
#include <netuno/nir/module.h>

#define IS_TYPE(obj, ptype) (((NT_OBJECT *)(obj))->type == (ptype))
#define IS_VALID_TYPE(ptype)                                                                       \
    ((ptype) && ((NT_TYPE *)(ptype))->objectType >= NT_TYPE_MIN &&                                 \
     ((NT_TYPE *)(ptype))->objectType <= NT_TYPE_MAX)

typedef enum
{
    NT_TYPE_ERROR,
    NT_TYPE_UNDEFINED,
    NT_TYPE_VOID,
    NT_TYPE_STRING,
    NT_TYPE_F64,
    NT_TYPE_F32,
    NT_TYPE_U64,
    NT_TYPE_I64,
    NT_TYPE_U32,
    NT_TYPE_I32,
    NT_TYPE_BOOL,
    NT_TYPE_DELEGATE,
    NT_TYPE_ASSEMBLY,
    NT_TYPE_MODULE,
    NT_TYPE_OBJECT,
    NT_TYPE_CUSTOM,

    NT_TYPE_MIN = NT_TYPE_ERROR,
    NT_TYPE_MAX = NT_TYPE_CUSTOM,
} NT_TYPE_ID;

typedef struct _NT_TYPE NT_TYPE;

typedef struct _NT_PARAM
{
    const NT_TYPE *type;
    NT_STRING *name;
} NT_PARAM;

struct _NT_TYPE
{
    NT_TYPE_ID objectType;
    NT_STRING *typeName;
    const NT_TYPE *baseType;
    NT_SCOPE fields;

    union {
        struct
        {
            const NT_TYPE *returnType;
            size_t paramCount;
            NT_PARAM *params;
        } delegate;
        struct
        {
            NIR_MODULE *nirModule;
        } module;
    };
};

bool ntTypeIsAssignableFrom(const NT_TYPE *to, const NT_TYPE *from);

// const NT_TYPE *ntBoolType(void);
// const NT_TYPE *ntI32Type(void);
// const NT_TYPE *ntI64Type(void);
// const NT_TYPE *ntU32Type(void);
// const NT_TYPE *ntU64Type(void);
// const NT_TYPE *ntF32Type(void);
// const NT_TYPE *ntF64Type(void);
const NT_TYPE *ntStringType(NIR_CONTEXT *context);
const NT_TYPE *ntObjectType(void);

const NT_TYPE *ntUndefinedType(void);
const NT_TYPE *ntVoidType(void);
const NT_TYPE *ntErrorType(void);

NT_TYPE *ntTakeType(NT_TYPE_ID objectType, NT_STRING *typeName, const NT_TYPE *baseType);
const NT_TYPE *ntTakeDelegateType(const NT_TYPE *returnType, size_t paramCount,
                                  const NT_PARAM *params);
// char_t *ntDelegateTypeName(const NT_TYPE *returnType, size_t paramCount, const NT_PARAM *params);
// const NT_TYPE *ntCreateDelegateType(NT_STRING *delegateTypeName, const NT_TYPE *returnType,
//                                     size_t paramCount, const NT_PARAM *params);

#endif
