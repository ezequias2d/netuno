#include <assert.h>
#include <math.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <stdio.h>

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
        STRING_TYPE.typeName = ntCopyString(U"string", 6);
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

uint32_t ntStringToU32(const NT_STRING *string)
{
    const char_t *str = string->chars;
    uint32_t value = 0;
    size_t i_char = 0;

    while (str[i_char] == ' ')
        ++i_char;

    while (str[i_char] >= '0' && str[i_char] <= '9')
    {
        const uint32_t k = UINT32_MAX - (UINT32_MAX / 10) * 10;
        if (value > UINT32_MAX / 10 || (value == UINT32_MAX / 10 && str[i_char] - '0' > k))
            return UINT32_MAX;
        value = 10 * value + (str[i_char++] - '0');
    }
    return value;
}

uint64_t ntStringToU64(const NT_STRING *string)
{
    const char_t *str = string->chars;
    uint64_t value = 0;
    size_t i_char = 0;

    while (str[i_char] == ' ')
        ++i_char;

    while (str[i_char] >= '0' && str[i_char] <= '9')
    {
        const uint32_t k = UINT64_MAX - (UINT64_MAX / 10) * 10;
        if (value > UINT64_MAX / 10 || (value == UINT64_MAX / 10 && str[i_char] - '0' > k))
            return UINT64_MAX;
        value = 10 * value + (str[i_char++] - '0');
    }
    return value;
}

uint32_t ntStringToI32(const NT_STRING *string)
{
    const char_t *str = string->chars;
    int32_t value = 0;
    int32_t sign = 1;
    size_t i_char = 0;

    while (str[i_char] == ' ')
        ++i_char;

    if (str[i_char] == '-' || str[i_char] == '+')
        sign = 1 - 2 * (str[i_char] == '-');

    while (str[i_char] >= '0' && str[i_char] <= '9')
    {
        const uint32_t k = INT32_MAX - (INT32_MAX / 10) * 10;
        if (value > INT32_MAX / 10 || (value == INT32_MAX / 10 && str[i_char] - '0' > k))
        {
            if (sign == 1)
            {
                value = INT32_MAX;
                sign = 1;
            }
            else
            {
                value = INT32_MIN;
                sign = 1;
            }
        }
        value = 10 * value + (str[i_char++] - '0');
    }
    value = value * sign;
    return *(uint32_t *)&value;
}

uint64_t ntStringToI64(const NT_STRING *string)
{
    const char_t *str = string->chars;
    int64_t value = 0;
    int64_t sign = 1;
    size_t i_char = 0;

    while (str[i_char] == ' ')
        ++i_char;

    if (str[i_char] == '-' || str[i_char] == '+')
        sign = 1 - 2 * (str[i_char] == '-');

    while (str[i_char] >= '0' && str[i_char] <= '9')
    {
        const int64_t k = INT64_MAX - (INT64_MAX / 10) * 10;
        if (value > INT64_MAX / 10 || (value == INT64_MAX / 10 && str[i_char] - '0' > k))
        {
            if (sign == 1)
            {
                value = INT64_MAX;
                sign = 1;
            }
            else
            {
                value = INT64_MIN;
                sign = 1;
            }
        }
        value = 10 * value + (str[i_char++] - '0');
    }
    value = value * sign;
    return *(uint64_t *)&value;
}

uint32_t ntStringToF32(const NT_STRING *string)
{
    char *str = ntToCharFixed(string->chars, string->length);
    float value;
    int result = sscanf(str, "%f", &value);
    ntFree(str);

    if (result == EOF || result == 0)
        value = NAN;
    return *(uint32_t *)&value;
}

uint64_t ntStringToF64(const NT_STRING *string)
{
    char *str = ntToCharFixed(string->chars, string->length);
    double value;
    int result = sscanf(str, "%lf", &value);
    ntFree(str);

    if (result == EOF || result == 0)
        value = NAN;
    return *(uint64_t *)&value;
}
