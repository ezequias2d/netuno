#ifndef NETUNO_ARRAY_H
#define NETUNO_ARRAY_H

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
size_t ntArrayAddVarint(NT_ARRAY *array, const uint64_t value);

void ntArrayInsert(NT_ARRAY *array, size_t offset, const void *data, size_t dataSize);
void ntArrayInsertVarint(NT_ARRAY *array, size_t offset, const uint64_t value);

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
