#include <assert.h>
#include <netuno/memory.h>
#include <netuno/object.h>
#include <netuno/str.h>
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

static NT_TABLE stringTable = {.count = 0, .size = 0, .pEntries = NULL};

static void freeString(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_STRING);
    NT_STRING *string = (NT_STRING *)object;
    ntFree(string->chars);
    string->chars = NULL;
    string->length = 0;
}

static const NT_STRING *stringToString(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_STRING);
    object->refCount++;
    return (const NT_STRING *)object;
}

static bool stringEquals(NT_OBJECT *_str1, NT_OBJECT *_str2)
{
    assert(_str1->type->objectType == NT_OBJECT_STRING);
    assert(_str2->type->objectType == NT_OBJECT_STRING);
    NT_STRING *str1 = (NT_STRING *)_str1;
    NT_STRING *str2 = (NT_STRING *)_str2;

    if (str1->hash != str2->hash)
        return false;
    if (str1->length != str2->length)
        return false;
    return ntStrEqualsFixed(str1->chars, str1->length * sizeof(char_t), str2->chars,
                            str2->length * sizeof(char_t));
}

static NT_TYPE STRING_TYPE = {
    .objectType = NT_OBJECT_STRING,
    .typeName = NULL,
    .free = freeString,
    .string = stringToString,
    .equals = stringEquals,
    sizeof(uint32_t),
    sizeof(NT_STRING),
};

const NT_TYPE *ntStringType(void)
{
    if (STRING_TYPE.typeName == NULL)
        STRING_TYPE.typeName = ntCopyString(U"string", 6 * sizeof(char_t));
    return &STRING_TYPE;
}

static NT_STRING *allocString(char_t *chars, const size_t length, const uint32_t hash)
{
    NT_STRING *string = (NT_STRING *)ntCreateObject(&STRING_TYPE);
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    ntTableSet(&stringTable, string, NULL);
    return string;
}

static uint32_t hashString(const char_t *chars, const size_t length)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= chars[i];
        hash *= 16777619;
    }
    return hash;
}

const NT_STRING *ntCopyString(const char_t *chars, const size_t length)
{
    const uint32_t hash = hashString(chars, length);

    const NT_STRING *interned = ntTableFindString(&stringTable, chars, length, hash);
    if (interned != NULL)
        return interned;

    char_t *copyChars = (char_t *)ntMalloc((length + 1) * sizeof(char_t));
    ntMemcpy(copyChars, chars, length * sizeof(char_t));
    copyChars[length] = '\0';

    return allocString(copyChars, length, hash);
}

const NT_STRING *ntTakeString(char_t *chars, const size_t length)
{
    uint32_t hash = hashString(chars, length);
    const NT_STRING *interned = ntTableFindString(&stringTable, chars, length, hash);
    if (interned != NULL)
    {
        ntFree(chars);
        return interned;
    }

    return allocString(chars, length, hash);
}

const NT_STRING *ntConcat(NT_OBJECT *object1, NT_OBJECT *object2)
{
    const NT_STRING *str1 = ntToString(object1);
    const NT_STRING *str2 = ntToString(object2);

    size_t length = str1->length + str2->length;
    char_t *chars = (char_t *)ntMalloc(sizeof(char_t) * length);

    ntMemcpy(chars, str1->chars, str1->length * sizeof(char_t));
    ntMemcpy(chars + str1->length, str2->chars, str2->length * sizeof(char_t));

    ntFreeObject((NT_OBJECT *)str1);
    ntFreeObject((NT_OBJECT *)str2);

    NT_STRING *result = (NT_STRING *)ntCreateObject(&STRING_TYPE);
    result->chars = chars;
    result->length = length;
    return result;
}

bool ntStrEquals(const char_t *str1, const char_t *str2)
{
    for (size_t i = 0; str1[i] != '\0' || str2[i] != '\0'; ++i)
        if (str1[i] != str2[i])
            return false;
    return true;
}

bool ntStrEqualsFixed(const char_t *str1, const size_t size1, const char_t *str2,
                      const size_t size2)
{
    if (size1 != size2)
        return false;

    for (size_t i = 0; i < size1; ++i)
        if (str1[i] != str2[i])
            return false;
    return true;
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
    assert(snprintf(number, length + 1, "%f", value) == length);
    char_t *chars = ntToCharTFixed(number, length);
    ntFree(number);

    return ntTakeString(chars, length);
}

static const NT_STRING *f64ToString(NT_OBJECT *object)
{
    const double value = *(double *)object;
    const int length = snprintf(NULL, 0, "%lf", value);

    char *number = (char *)ntMalloc(sizeof(char) * (length + 1));
    assert(snprintf(number, length + 1, "%lf", value) == length);
    char_t *chars = ntToCharTFixed(number, length);
    ntFree(number);

    return ntTakeString(chars, length);
}

static const NT_STRING *delegateToString(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_FUNCTION);
    const NT_FUNCTION *function = (const NT_FUNCTION *)object;
    return function->name;
}

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    return obj1 == obj2;
}

char_t *ntDelegateTypeName(const NT_TYPE *returnType, size_t paramCount, const NT_PARAM *params)
{
    NT_ARRAY array;
    ntInitArray(&array);

    const char_t *const delegate = U"delegate(";
    ntArrayAdd(&array, delegate, ntStrLen(delegate));
    for (size_t i_param = 0; i_param < paramCount; ++i_param)
    {
        const NT_PARAM *param = &params[i_param];
        assert(param->type);
        const NT_STRING *typeName = param->type->typeName;
        ntArrayAdd(&array, typeName->chars, typeName->length * sizeof(char_t));
    }
    ntArrayAdd(&array, L")", sizeof(char_t));

    if (returnType)
    {
        assert(returnType->typeName);
        ntArrayAdd(&array, L":", sizeof(char_t));
        ntArrayAdd(&array, returnType->typeName->chars, returnType->typeName->length);
    }

    const char_t term = '\0';
    ntArrayAdd(&array, &term, sizeof(char_t));

    return ntRealloc(array.data, array.count);
}

NT_TYPE *ntCreateDelegateType(const NT_STRING *name, const NT_TYPE *returnType, size_t paramCount,
                              const NT_PARAM *params)
{
    NT_DELEGATE_TYPE *type =
        (NT_DELEGATE_TYPE *)ntMalloc(sizeof(NT_DELEGATE_TYPE) + sizeof(NT_PARAM) * paramCount);

    *type = (NT_DELEGATE_TYPE){
        .type =
            (NT_TYPE){
                .objectType = NT_OBJECT_FUNCTION,
                .typeName = name,
                .free = freeNone,
                .string = delegateToString,
                .equals = refEquals,
                .stackSize = sizeof(uint32_t),
                .instanceSize = 0,
            },
        .paramCount = paramCount,
        .returnType = returnType,
    };
    ntMemcpy((void *)type->params, params, sizeof(NT_PARAM) * paramCount);
    return (NT_TYPE *)type;
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
        I32_TYPE.typeName = ntCopyString(U"i32", 3 * sizeof(char_t));
    return &I32_TYPE;
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
        I64_TYPE.typeName = ntCopyString(U"i64", 3 * sizeof(char_t));
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
        U32_TYPE.typeName = ntCopyString(U"u32", 3 * sizeof(char_t));
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
        U64_TYPE.typeName = ntCopyString(U"u64", 3 * sizeof(char_t));
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
        F32_TYPE.typeName = ntCopyString(U"f32", 3 * sizeof(char_t));
    return &U32_TYPE;
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
        F64_TYPE.typeName = ntCopyString(U"f64", 3 * sizeof(char_t));
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
        ERROR_TYPE.typeName = ntCopyString(U"error", 3 * sizeof(char_t));
    return &ERROR_TYPE;
}
