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

#include "netuno/symbol.h"
#include <netuno/common.h>
#include <netuno/object.h>

#define IS_TYPE(obj, type) (((NT_OBJECT *)(obj))->type == (type))
#define IS_VALID_TYPE(type)                                                                        \
    ((type) && ((NT_TYPE *)(type))->objectType >= NT_OBJECT_ERROR &&                               \
     ((NT_TYPE *)(type))->objectType <= NT_OBJECT_TYPE_MAX)

typedef enum
{
    NT_OBJECT_ERROR,
    NT_OBJECT_UNDEFINED,
    NT_OBJECT_VOID,
    NT_OBJECT_STRING,
    NT_OBJECT_F64,
    NT_OBJECT_F32,
    NT_OBJECT_U64,
    NT_OBJECT_I64,
    NT_OBJECT_U32,
    NT_OBJECT_I32,
    NT_OBJECT_DELEGATE,
    NT_OBJECT_ASSEMBLY,
    NT_OBJECT_MODULE,
    NT_OBJECT_OBJECT,
    NT_OBJECT_TYPE_TYPE,
    NT_OBJECT_CUSTOM,

    NT_OBJECT_TYPE_MIN = NT_OBJECT_ERROR,
    NT_OBJECT_TYPE_MAX = NT_OBJECT_CUSTOM,
} NT_OBJECT_TYPE;

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_STRING NT_STRING;
typedef struct _NT_TYPE NT_TYPE;

typedef void (*freeObj)(NT_OBJECT *obj);
typedef const NT_STRING *(*toString)(NT_OBJECT *obj);
typedef bool (*equalsObj)(NT_OBJECT *obj1, NT_OBJECT *obj2);

struct _NT_TYPE
{
    NT_OBJECT object;
    NT_OBJECT_TYPE objectType;
    const NT_STRING *typeName;
    freeObj free;
    toString string;
    equalsObj equals;
    size_t stackSize;
    size_t instanceSize;
    const NT_TYPE *baseType;
    NT_SYMBOL_TABLE fields;
};

const NT_TYPE *ntType(void);
bool ntTypeIsAssignableFrom(const NT_TYPE *to, const NT_TYPE *from);

#endif
