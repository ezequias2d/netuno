#ifndef NT_STRING_H
#define NT_STRING_H

#include "object.h"
#include "type.h"

typedef struct _NT_STRING NT_STRING;
struct _NT_STRING
{
    NT_OBJECT object;
    size_t length;
    char_t *chars;
    uint32_t hash;
};

const NT_TYPE *ntStringType(void);
const NT_STRING *ntCopyString(const char_t *chars, const size_t length);
const NT_STRING *ntTakeString(char_t *chars, const size_t length);
bool ntStrEquals(const char_t *str1, const char_t *str2);
bool ntStrEqualsFixed(const char_t *str1, const size_t size1, const char_t *str2,
                      const size_t size2);

uint32_t ntStringToU32(const NT_STRING *string);
uint64_t ntStringToU64(const NT_STRING *string);
uint32_t ntStringToI32(const NT_STRING *string);
uint64_t ntStringToI64(const NT_STRING *string);
uint32_t ntStringToF32(const NT_STRING *string);
uint64_t ntStringToF64(const NT_STRING *string);

#endif
