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
#ifndef NT_STRING_H
#define NT_STRING_H

#include <netuno/common.h>
#include <netuno/object.h>

typedef struct _NT_TYPE NT_TYPE;
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
