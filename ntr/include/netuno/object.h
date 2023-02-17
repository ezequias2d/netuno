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
#ifndef NT_OBJECT_H
#define NT_OBJECT_H

#include <netuno/array.h>
#include <netuno/common.h>
#include <netuno/symbol.h>
#include <netuno/table.h>

#define IS_VALID_OBJECT(obj) ((obj) && IS_VALID_TYPE(((NT_OBJECT *)(obj))->type))

typedef struct _NT_VM NT_VM;
typedef struct _NT_MODULE NT_MODULE;

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_TYPE NT_TYPE;
typedef struct _NT_DELEGATE_TYPE NT_DELEGATE_TYPE;
typedef struct _NT_STRING NT_STRING;

typedef bool (*nativeFun)(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType);

struct _NT_OBJECT
{
    const NT_TYPE *type;
    size_t refCount;
};

const NT_TYPE *ntObjectType(void);
NT_OBJECT *ntCreateObject(const NT_TYPE *type);
void ntRefObject(NT_OBJECT *object);
void ntFreeObject(NT_OBJECT *object);
void ntMakeConstant(NT_OBJECT *object);
void ntForceFreeObject(NT_OBJECT *object);
const NT_STRING *ntToString(NT_OBJECT *object);
bool ntEquals(NT_OBJECT *object1, NT_OBJECT *object2);
const NT_STRING *ntConcat(NT_OBJECT *object1, NT_OBJECT *object2);

const NT_TYPE *ntBoolType(void);
const NT_TYPE *ntI32Type(void);
const NT_TYPE *ntI64Type(void);
const NT_TYPE *ntU32Type(void);
const NT_TYPE *ntU64Type(void);
const NT_TYPE *ntF32Type(void);
const NT_TYPE *ntF64Type(void);

const NT_TYPE *ntUndefinedType(void);
const NT_TYPE *ntVoidType(void);
const NT_TYPE *ntErrorType(void);

#endif
