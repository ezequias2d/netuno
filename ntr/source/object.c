#include "netuno/type.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <stdio.h>
#include <string.h>

NT_OBJECT *ntCreateObject(const NT_TYPE *type)
{
    NT_OBJECT *object = (NT_OBJECT *)ntMalloc(type->instanceSize);
    object->type = type;
    object->refCount = 1;
    return object;
}

void ntRef(NT_OBJECT *object)
{
    assert(object->refCount);
    object->refCount++;
}

void ntFreeObject(NT_OBJECT *object)
{
    assert(object->refCount);
    object->refCount--;
    if (object->refCount <= 0)
    {
        object->type->free(object);
        ntFree(object);
    }
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
    .objectType = NT_OBJECT_I32,
    .typeName = NULL,
    .free = freeNone,
    .string = i32ToString,
    .equals = NULL,
    .stackSize = sizeof(int32_t),
    .instanceSize = 0,
};

const NT_TYPE *ntI32Type(void)
{
    if (I32_TYPE.typeName == NULL)
        I32_TYPE.typeName = ntCopyString(U"int", 3);
    return &I32_TYPE;
}

const NT_TYPE *ntBoolType(void)
{
    return ntI32Type();
}

static NT_TYPE I64_TYPE = {
    .objectType = NT_OBJECT_I64,
    .typeName = NULL,
    .free = freeNone,
    .string = i64ToString,
    .equals = NULL,
    .stackSize = sizeof(int64_t),
    .instanceSize = 0,
};

const NT_TYPE *ntI64Type(void)
{
    if (I64_TYPE.typeName == NULL)
        I64_TYPE.typeName = ntCopyString(U"long", 4);
    return &I64_TYPE;
}

static NT_TYPE U32_TYPE = {
    .objectType = NT_OBJECT_U32,
    .typeName = NULL,
    .free = freeNone,
    .string = u32ToString,
    .equals = NULL,
    .stackSize = sizeof(uint32_t),
    .instanceSize = 0,
};

const NT_TYPE *ntU32Type(void)
{
    if (U32_TYPE.typeName == NULL)
        U32_TYPE.typeName = ntCopyString(U"uint", 4);
    return &U32_TYPE;
}

static NT_TYPE U64_TYPE = {
    .objectType = NT_OBJECT_U64,
    .typeName = NULL,
    .free = freeNone,
    .string = u64ToString,
    .equals = NULL,
    .stackSize = sizeof(uint64_t),
    .instanceSize = 0,
};

const NT_TYPE *ntU64Type(void)
{
    if (U64_TYPE.typeName == NULL)
        U64_TYPE.typeName = ntCopyString(U"ulong", 5);
    return &U64_TYPE;
}

static NT_TYPE F32_TYPE = {
    .objectType = NT_OBJECT_F32,
    .typeName = NULL,
    .free = freeNone,
    .string = f32ToString,
    .equals = NULL,
    .stackSize = sizeof(float),
    .instanceSize = 0,
};

const NT_TYPE *ntF32Type(void)
{
    if (F32_TYPE.typeName == NULL)
        F32_TYPE.typeName = ntCopyString(U"float", 5);
    return &F32_TYPE;
}

static NT_TYPE F64_TYPE = {
    .objectType = NT_OBJECT_F64,
    .typeName = NULL,
    .free = freeNone,
    .string = f64ToString,
    .equals = NULL,
    .stackSize = sizeof(double),
    .instanceSize = 0,
};

const NT_TYPE *ntF64Type(void)
{
    if (F64_TYPE.typeName == NULL)
        F64_TYPE.typeName = ntCopyString(U"double", 6);
    return &F64_TYPE;
}

static const NT_STRING *errorToString(NT_OBJECT *object)
{
    assert(object);
    const char_t str[] = U"{ERROR TYPE}";
    return ntCopyString(str, sizeof(str) / sizeof(char_t));
}

static NT_TYPE ERROR_TYPE = {
    .objectType = NT_OBJECT_ERROR,
    .typeName = NULL,
    .free = freeNone,
    .string = errorToString,
    .equals = NULL,
    .stackSize = 0,
    .instanceSize = 0,
};

const NT_TYPE *ntErrorType(void)
{
    if (ERROR_TYPE.typeName == NULL)
        ERROR_TYPE.typeName = ntCopyString(U"error", 3);
    return &ERROR_TYPE;
}
