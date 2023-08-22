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
#include "pstring.h"
#include <assert.h>
#include <math.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <stdio.h>

static NT_TABLE stringTable = {.count = 0, .size = 0, .pEntries = NULL};

const char_t *ntStringChars(const NT_STRING *str, size_t *length)
{
    if (length)
        *length = str->length;
    return str->chars;
}

uint32_t ntStringHash(const NT_STRING *str)
{
    assert(str);
    return str->hash;
}

static void freeString(NT_STRING *string)
{
    ntFree(string->chars);
    string->chars = NULL;
    string->length = 0;
}

bool ntStringEquals(const NT_STRING *str1, const NT_STRING *str2)
{
    if (str1->hash != str2->hash)
        return false;
    if (str1->length != str2->length)
        return false;
    return ntStrEqualsFixed(str1->chars, str1->length, str2->chars,
                            str2->length);
}

bool ntStringEquals2(const NT_STRING *str1, const char_t *str2,
                     size_t str2Length)
{
    if (str1->length != str2Length)
        return false;
    return ntStrEqualsFixed(str1->chars, str1->length, str2, str2Length);
}

static NT_STRING *allocString(char_t *chars, const size_t length,
                              const uint32_t hash)
{
    NT_STRING *string = (NT_STRING *)ntMalloc(sizeof(NT_STRING));
    string->refCount = 1;
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    ntTableSet(&stringTable, string, NULL);
    return string;
}

NT_STRING *ntRefString(const NT_STRING *_string)
{
    NT_STRING *const string = (NT_STRING *)_string;
    if (string->refCount)
        string->refCount++;
    return string;
}

void ntUnrefString(NT_STRING *string)
{
    // refCount == 0 for constant objects
    if (string && string->refCount > 0)
    {
        string->refCount--;
        if (string->refCount <= 0)
        {
            freeString(string);
        }
    }
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

NT_STRING *ntCopyString(const char_t *chars, const size_t length)
{
    const uint32_t hash = hashString(chars, length);

    NT_STRING *interned = ntTableFindString(&stringTable, chars, length, hash);
    if (interned != NULL)
    {
        ntRefString(interned);
        return interned;
    }

    char_t *copyChars = (char_t *)ntMalloc((length + 1) * sizeof(char_t));
    ntMemcpy(copyChars, chars, length * sizeof(char_t));
    copyChars[length] = '\0';

    return allocString(copyChars, length, hash);
}

NT_STRING *ntCopyCString(const char_t *chars)
{
    return ntCopyString(chars, ntStrLen(chars));
}

NT_STRING *ntTakeString(char_t *chars, const size_t length)
{
    uint32_t hash = hashString(chars, length);
    NT_STRING *interned = ntTableFindString(&stringTable, chars, length, hash);
    if (interned != NULL)
    {
        ntFree(chars);
        return interned;
    }

    return allocString(chars, length, hash);
}

size_t ntStringLength(const NT_STRING *str1)
{
    assert(str1);
    return str1->length;
}

NT_STRING *ntConcat(const NT_STRING *str1, const NT_STRING *str2)
{
    size_t length = str1->length + str2->length;
    char_t *chars = (char_t *)ntMalloc(sizeof(char_t) * length);

    ntMemcpy(chars, str1->chars, str1->length * sizeof(char_t));
    ntMemcpy(chars + str1->length, str2->chars, str2->length * sizeof(char_t));

    return ntTakeString(chars, length);
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
        if (value > UINT32_MAX / 10 ||
            (value == UINT32_MAX / 10 && str[i_char] - '0' > k))
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
        if (value > UINT64_MAX / 10 ||
            (value == UINT64_MAX / 10 && str[i_char] - '0' > k))
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
        if (value > INT32_MAX / 10 ||
            (value == INT32_MAX / 10 && str[i_char] - '0' > k))
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
        if (value > INT64_MAX / 10 ||
            (value == INT64_MAX / 10 && str[i_char] - '0' > k))
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

const NT_STRING *ntI32ToString(int32_t value)
{
    char number[11];
    const size_t length = sprintf(number, "%d", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

const NT_STRING *ntI64ToString(int64_t value)
{
    char number[20];
    const size_t length = sprintf(number, "%ld", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

const NT_STRING *ntU32ToString(uint32_t value)
{
    char number[10];
    const size_t length = sprintf(number, "%u", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

const NT_STRING *ntU64ToString(uint64_t value)
{
    char number[20];
    const size_t length = sprintf(number, "%lu", value);

    char_t *chars = ntToCharTFixed(number, length);
    return ntTakeString(chars, length);
}

const NT_STRING *ntF32ToString(float value)
{
    const int length = snprintf(NULL, 0, "%f", value);

    char *number = (char *)ntMalloc(sizeof(char) * (length + 1));
    const bool result = snprintf(number, length + 1, "%f", value) == length;
    assert(result);
    char_t *chars = ntToCharTFixed(number, length);
    ntFree(number);

    return ntTakeString(chars, length);
}

const NT_STRING *ntF64ToString(double value)
{
    const int length = snprintf(NULL, 0, "%lf", value);

    char *number = (char *)ntMalloc(sizeof(char) * (length + 1));
    const bool result = snprintf(number, length + 1, "%lf", value) == length;
    assert(result);
    char_t *chars = ntToCharTFixed(number, length);
    ntFree(number);

    return ntTakeString(chars, length);
}

void ntPrintString(const NT_STRING *string)
{
    char *str = ntStringToChar(string);
    printf("%s", str);
    ntFree(str);
}
