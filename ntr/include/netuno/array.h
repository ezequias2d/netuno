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
#ifndef NT_ARRAY_H
#define NT_ARRAY_H

#include "common.h"

typedef struct _NT_ARRAY
{
    size_t size;
    size_t count;
    uint8_t *data;
} NT_ARRAY;

NT_ARRAY *ntCreateArray(void);
void ntInitArray(NT_ARRAY *array);
void ntDeinitArray(NT_ARRAY *array);
void ntFreeArray(NT_ARRAY *array);
void ntArraySet(NT_ARRAY *array, size_t offset, const void *data, size_t dataSize);

size_t ntArrayAdd(NT_ARRAY *array, const void *data, size_t dataSize);
size_t ntArrayAddF32(NT_ARRAY *array, const float value);
size_t ntArrayAddF64(NT_ARRAY *array, const double value);
size_t ntArrayAddI32(NT_ARRAY *array, const int32_t value);
size_t ntArrayAddU32(NT_ARRAY *array, const uint32_t value);
size_t ntArrayAddI64(NT_ARRAY *array, const int64_t value);
size_t ntArrayAddU64(NT_ARRAY *array, const uint64_t value);
size_t ntArrayAddVarint(NT_ARRAY *array, const uint64_t value, size_t *size);

void ntArrayInsert(NT_ARRAY *array, size_t offset, const void *data, size_t dataSize);
size_t ntArrayInsertVarint(NT_ARRAY *array, size_t offset, const uint64_t value);

size_t ntArrayGet(const NT_ARRAY *array, const size_t offset, void *data, size_t dataSize);
void ntArrayGetString(const NT_ARRAY *array, size_t offset, char_t *data, size_t *dataSize);
size_t ntArrayGetF32(const NT_ARRAY *array, const size_t offset, float *value);
size_t ntArrayGetF64(const NT_ARRAY *array, const size_t offset, double *value);
size_t ntArrayGetI32(const NT_ARRAY *array, const size_t offset, int32_t *value);
size_t ntArrayGetU32(const NT_ARRAY *array, const size_t offset, uint32_t *value);
size_t ntArrayGetI64(const NT_ARRAY *array, const size_t offset, int64_t *value);
size_t ntArrayGetU64(const NT_ARRAY *array, const size_t offset, uint64_t *value);
size_t ntArrayGetVarint(const NT_ARRAY *array, const size_t offset, uint64_t *value);

bool ntArrayFind(const NT_ARRAY *array, const void *data, const size_t dataSize, size_t *offset);

#endif
