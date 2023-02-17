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
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <netuno/type.h>
#include <stdio.h>
#include <string.h>

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    {
        assert(obj1);
        assert(IS_VALID_OBJECT(obj1));
        assert(obj1->type->objectType == NT_OBJECT_TYPE_TYPE);

        NT_TYPE *type = (NT_TYPE *)obj1;
        assert(IS_VALID_TYPE(type));
    }
    {
        assert(obj2);
        assert(IS_VALID_OBJECT(obj2));
        assert(obj2->type->objectType == NT_OBJECT_TYPE_TYPE);

        NT_TYPE *type = (NT_TYPE *)obj2;
        assert(IS_VALID_TYPE(type));
    }

    return obj1 == obj2;
}

static void freeType(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_TYPE_TYPE);

    NT_TYPE *type = (NT_TYPE *)object;
    assert(IS_VALID_TYPE(type));

    ntFreeObject((NT_OBJECT *)type->typeName);
}

static const NT_STRING *typeToString(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_TYPE_TYPE);

    NT_TYPE *type = (NT_TYPE *)object;
    assert(IS_VALID_TYPE(type));

    return type->typeName;
}

static NT_TYPE TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_TYPE_TYPE,
    .typeName = NULL,
    .free = freeType,
    .string = typeToString,
    .equals = refEquals,
    .stackSize = sizeof(NT_REF),
    .instanceSize = sizeof(NT_TYPE),
    .baseType = NULL,
};

const NT_TYPE *ntType(void)
{
    if (TYPE.object.type == NULL)
    {
        TYPE.object.type = &TYPE;
        TYPE.typeName = ntCopyString(U"Type", 4);
        TYPE.baseType = ntObjectType();
        ntInitSymbolTable(&TYPE.fields, (NT_SYMBOL_TABLE *)&ntObjectType()->fields, STT_TYPE, 0);
    }
    return &TYPE;
}

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

static void freeObject(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
}

static const NT_STRING *objectToString(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_TYPE_TYPE);

    const NT_STRING *addrString = NULL;
    switch (sizeof(NT_OBJECT *))
    {
    case sizeof(uint32_t):
        addrString = ntU32Type()->string((NT_OBJECT *)&object);
        break;
    default:
        addrString = ntU64Type()->string((NT_OBJECT *)&object);
        break;
    }

    const NT_STRING *className =
        ntConcat((NT_OBJECT *)object->type->typeName, (NT_OBJECT *)ntCopyString(U" ", 1));

    const NT_STRING *result = ntConcat((NT_OBJECT *)className, (NT_OBJECT *)addrString);

    ntFreeObject((NT_OBJECT *)addrString);
    ntFreeObject((NT_OBJECT *)className);

    return result;
}

static NT_TYPE OBJECT_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_TYPE_TYPE,
    .typeName = NULL,
    .free = freeObject,
    .string = objectToString,
    .equals = refEquals,
    .stackSize = sizeof(NT_REF),
    .instanceSize = sizeof(NT_OBJECT),
    .baseType = NULL,
};

const NT_TYPE *ntObjectType(void)
{
    if (OBJECT_TYPE.object.type == NULL)
    {
        OBJECT_TYPE.object.type = ntType();
        OBJECT_TYPE.typeName = ntCopyString(U"Object", 6);
        OBJECT_TYPE.baseType = NULL;
        ntInitSymbolTable(&OBJECT_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &OBJECT_TYPE;
}

NT_OBJECT *ntCreateObject(const NT_TYPE *type)
{
    NT_OBJECT *object = (NT_OBJECT *)ntMalloc(type->instanceSize);
    object->type = type;
    object->refCount = 1;
    return object;
}

void ntRefObject(NT_OBJECT *object)
{
    if (object->refCount)
        object->refCount++;
}

void ntFreeObject(NT_OBJECT *object)
{
    assert(object);
    // refCount == 0 for constant objects
    if (object->refCount > 0)
    {
        object->refCount--;
        if (object->refCount <= 0)
        {
            ntForceFreeObject(object);
        }
    }
}

void ntMakeConstant(NT_OBJECT *object)
{
    object->refCount = 0;
}

static void freeBase(NT_OBJECT *object, const NT_TYPE *current)
{
    if (current->baseType)
        freeBase(object, current->baseType);
    current->free(object);
}

void ntForceFreeObject(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));

    freeBase(object, object->type);
    ntFree(object);
}

const NT_STRING *ntToString(NT_OBJECT *object)
{
    return object->type->string(object);
}

bool ntEquals(NT_OBJECT *object1, NT_OBJECT *object2)
{
    return object1->type->equals(object1, object2);
}

static void freeNone(NT_OBJECT *object)
{
    assert(object);
}

static const NT_STRING *i32ToString(NT_OBJECT *object)
{
    const int32_t value = *(int32_t *)object;
    char number[11];
    const size_t length = sprintf(number, "%d", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

static const NT_STRING *i64ToString(NT_OBJECT *object)
{
    const int64_t value = *(int64_t *)object;
    char number[20];
    const size_t length = sprintf(number, "%ld", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

static const NT_STRING *u32ToString(NT_OBJECT *object)
{
    const uint32_t value = *(uint32_t *)object;
    char number[10];
    const size_t length = sprintf(number, "%u", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

static const NT_STRING *u64ToString(NT_OBJECT *object)
{
    const uint64_t value = *(uint64_t *)object;
    char number[20];
    const size_t length = sprintf(number, "%lu", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

static const NT_STRING *f32ToString(NT_OBJECT *object)
{
    const float value = *(float *)object;
    const int length = snprintf(NULL, 0, "%f", value);

    char *number = (char *)ntMalloc(sizeof(char) * (length + 1));
    const bool result = snprintf(number, length + 1, "%f", value) == length;
    assert(result);
    char_t *chars = ntToCharTFixed(number, length);
    ntFree(number);

    return ntTakeString(chars, length);
}

static const NT_STRING *f64ToString(NT_OBJECT *object)
{
    const double value = *(double *)object;
    const int length = snprintf(NULL, 0, "%lf", value);

    char *number = (char *)ntMalloc(sizeof(char) * (length + 1));
    const bool result = snprintf(number, length + 1, "%lf", value) == length;
    assert(result);
    char_t *chars = ntToCharTFixed(number, length);
    ntFree(number);

    return ntTakeString(chars, length);
}

static NT_TYPE I32_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_I32,
    .typeName = NULL,
    .free = freeNone,
    .string = i32ToString,
    .equals = NULL,
    .stackSize = sizeof(int32_t),
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntI32Type(void)
{
    if (I32_TYPE.object.type == NULL)
    {
        I32_TYPE.object.type = ntType();
        I32_TYPE.typeName = ntCopyString(U"int", 3);
        ntInitSymbolTable(&I32_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &I32_TYPE;
}

const NT_TYPE *ntBoolType(void)
{
    return ntI32Type();
}

static NT_TYPE I64_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_I64,
    .typeName = NULL,
    .free = freeNone,
    .string = i64ToString,
    .equals = NULL,
    .stackSize = sizeof(int64_t),
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntI64Type(void)
{
    if (I64_TYPE.object.type == NULL)
    {
        I64_TYPE.object.type = ntType();
        I64_TYPE.typeName = ntCopyString(U"long", 4);
        ntInitSymbolTable(&I64_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &I64_TYPE;
}

static NT_TYPE U32_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_U32,
    .typeName = NULL,
    .free = freeNone,
    .string = u32ToString,
    .equals = NULL,
    .stackSize = sizeof(uint32_t),
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntU32Type(void)
{
    if (U32_TYPE.object.type == NULL)
    {
        U32_TYPE.object.type = ntType();
        U32_TYPE.typeName = ntCopyString(U"uint", 4);
        ntInitSymbolTable(&U32_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &U32_TYPE;
}

static NT_TYPE U64_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_U64,
    .typeName = NULL,
    .free = freeNone,
    .string = u64ToString,
    .equals = NULL,
    .stackSize = sizeof(uint64_t),
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntU64Type(void)
{
    if (U64_TYPE.object.type == NULL)
    {
        U64_TYPE.object.type = ntType();
        U64_TYPE.typeName = ntCopyString(U"ulong", 5);
        ntInitSymbolTable(&U64_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &U64_TYPE;
}

static NT_TYPE F32_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_F32,
    .typeName = NULL,
    .free = freeNone,
    .string = f32ToString,
    .equals = NULL,
    .stackSize = sizeof(float),
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntF32Type(void)
{
    if (F32_TYPE.object.type == NULL)
    {
        F32_TYPE.object.type = ntType();
        F32_TYPE.typeName = ntCopyString(U"float", 5);
        ntInitSymbolTable(&F32_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &F32_TYPE;
}

static NT_TYPE F64_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_F64,
    .typeName = NULL,
    .free = freeNone,
    .string = f64ToString,
    .equals = NULL,
    .stackSize = sizeof(double),
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntF64Type(void)
{
    if (F64_TYPE.object.type == NULL)
    {
        F64_TYPE.object.type = ntType();
        F64_TYPE.typeName = ntCopyString(U"double", 6);
        ntInitSymbolTable(&F64_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &F64_TYPE;
}

static const NT_STRING *undefinedToString(NT_OBJECT *object)
{
    assert(object);
    const char_t str[] = U"{UNDEFINED TYPE}";
    return ntCopyString(str, sizeof(str) / sizeof(char_t));
}

static NT_TYPE UNDEFINED_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_UNDEFINED,
    .typeName = NULL,
    .free = freeNone,
    .string = undefinedToString,
    .equals = NULL,
    .stackSize = 0,
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntUndefinedType(void)
{
    if (UNDEFINED_TYPE.object.type == NULL)
    {
        UNDEFINED_TYPE.object.type = ntType();
        UNDEFINED_TYPE.typeName = ntCopyString(U"undefined", 4);
        ntInitSymbolTable(&UNDEFINED_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &UNDEFINED_TYPE;
}

static const NT_STRING *voidToString(NT_OBJECT *object)
{
    assert(object);
    const char_t str[] = U"{VOID TYPE}";
    return ntCopyString(str, sizeof(str) / sizeof(char_t));
}

static NT_TYPE VOID_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_VOID,
    .typeName = NULL,
    .free = freeNone,
    .string = voidToString,
    .equals = NULL,
    .stackSize = 0,
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntVoidType(void)
{
    if (VOID_TYPE.object.type == NULL)
    {
        VOID_TYPE.object.type = ntType();
        VOID_TYPE.typeName = ntCopyString(U"void", 4);
        ntInitSymbolTable(&VOID_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &VOID_TYPE;
}

static const NT_STRING *errorToString(NT_OBJECT *object)
{
    assert(object);
    const char_t str[] = U"{ERROR TYPE}";
    return ntCopyString(str, sizeof(str) / sizeof(char_t));
}

static NT_TYPE ERROR_TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_ERROR,
    .typeName = NULL,
    .free = freeNone,
    .string = errorToString,
    .equals = NULL,
    .stackSize = 0,
    .instanceSize = 0,
    .baseType = NULL,
};

const NT_TYPE *ntErrorType(void)
{
    if (ERROR_TYPE.object.type == NULL)
    {
        ERROR_TYPE.object.type = ntType();
        ERROR_TYPE.typeName = ntCopyString(U"error", 3);
        ntInitSymbolTable(&ERROR_TYPE.fields, NULL, STT_TYPE, 0);
    }
    return &ERROR_TYPE;
}
