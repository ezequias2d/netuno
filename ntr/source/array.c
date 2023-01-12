#include <netuno/array.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/varint.h>
#include <string.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

NT_ARRAY *ntCreateArray(void)
{
    NT_ARRAY *array = (NT_ARRAY *)ntMalloc(sizeof(NT_ARRAY));
    ntInitArray(array);
    return array;
}

void ntInitArray(NT_ARRAY *array)
{
    array->size = 0;
    array->count = 0;
    array->data = NULL;
}

void ntDeinitArray(NT_ARRAY *array)
{
    array->size = 0;
    array->count = 0;
    if (array->data)
        ntFree(array->data);
}

void ntFreeArray(NT_ARRAY *array)
{
    if (array)
    {
        ntDeinitArray(array);
        ntFree(array);
    }
}

void ntArraySet(NT_ARRAY *array, size_t offset, const void *data, size_t dataSize)
{
    if (dataSize == 0)
        return;

    if (offset + dataSize > array->size)
    {
        const size_t newSize = MAX(array->size * 3 / 2, array->count + dataSize);
        array->data = ntRealloc(array->data, sizeof(uint8_t) * newSize);
        array->size = newSize;
    }
    ntMemcpy(array->data + offset, data, dataSize);
    array->count = MAX(offset + dataSize, array->count);
}

void ntArrayInsert(NT_ARRAY *array, size_t offset, const void *data, size_t dataSize)
{
    if (dataSize == 0)
        return;

    if (array->count + dataSize > array->size)
    {
        const size_t newSize = MAX(array->size * 3 / 2, array->count + dataSize);
        array->data = ntRealloc(array->data, sizeof(uint8_t) * newSize);
        array->size = newSize;
    }

    memmove(array->data + offset + dataSize, array->data + offset, array->count - offset);
    ntMemcpy(array->data + offset, data, dataSize);
    array->count += dataSize;
}

void ntArrayInsertVarint(NT_ARRAY *array, size_t offset, const uint64_t value)
{
    uint8_t tmp[10];
    const size_t size = ntEncodeVarint(tmp, sizeof(uint8_t) * 10, ZigZagEncoding(value));
    ntArrayInsert(array, offset, tmp, size);
}

size_t ntArrayAdd(NT_ARRAY *array, const void *data, size_t dataSize)
{
    if (dataSize == 0)
        return 0;

    if (array->count + dataSize > array->size)
    {
        const size_t newSize = MAX(array->size * 3 / 2, array->count + dataSize);
        array->data = ntRealloc(array->data, sizeof(uint8_t) * newSize);
        array->size = newSize;
    }
    ntMemcpy(array->data + array->count, data, dataSize);
    const size_t offset = array->count;
    array->count += dataSize;
    return offset;
}

size_t ntArrayAddVarint(NT_ARRAY *array, const uint64_t value)
{
    uint8_t *dst = array->data + array->count;
    size_t write = ntEncodeVarint(dst, array->size - array->count, ZigZagEncoding(value));
    if (write == 0)
    {
        const size_t newSize = MAX(array->size * 3 / 2, array->count + sizeof(uint64_t));
        array->data = ntRealloc(array->data, sizeof(uint8_t) * newSize);
        array->size = newSize;
        dst = array->data + array->count;
        write = ntEncodeVarint(dst, array->size - array->count, ZigZagEncoding(value));
    }
    const size_t offset = array->count;
    array->count += write;
    return offset;
}

size_t ntArrayAddF32(NT_ARRAY *array, const float value)
{
    return ntArrayAdd(array, &value, sizeof(value));
}

size_t ntArrayAddF64(NT_ARRAY *array, const double value)
{
    return ntArrayAdd(array, &value, sizeof(value));
}

size_t ntArrayAddI32(NT_ARRAY *array, const int32_t value)
{
    return ntArrayAdd(array, &value, sizeof(value));
}

size_t ntArrayAddU32(NT_ARRAY *array, const uint32_t value)
{
    return ntArrayAdd(array, &value, sizeof(value));
}

size_t ntArrayAddI64(NT_ARRAY *array, const int64_t value)
{
    return ntArrayAdd(array, &value, sizeof(value));
}

size_t ntArrayAddU64(NT_ARRAY *array, const uint64_t value)
{
    return ntArrayAdd(array, &value, sizeof(value));
}

size_t ntArrayGetVarint(const NT_ARRAY *array, const size_t offset, uint64_t *value)
{
    const uint8_t *src = array->data + offset;
    const size_t result = ntDecodeVarint(src, array->count - offset, value);
    if (value)
        *value = ZigZagDecoding(*value);
    return result;
}

size_t ntArrayGet(const NT_ARRAY *array, size_t offset, void *data, size_t dataSize)
{
    if (!(offset + dataSize <= array->count))
        return 0;

    ntMemcpy(data, array->data + offset, dataSize);
    return dataSize;
}

void ntArrayGetString(const NT_ARRAY *array, size_t offset, char_t *data, size_t *pLength)
{
    if (!(offset < array->count))
    {
        *pLength = 0;
        return;
    }

    const size_t length = ntStrLen((const char_t *)(array->data + offset));
    if (data && *pLength > 0)
    {
        ntArrayGet(array, offset, data, length * sizeof(char_t));
    }
    *pLength = length;
}

size_t ntArrayGetF32(const NT_ARRAY *array, const size_t offset, float *value)
{
    return ntArrayGet(array, offset, value, sizeof(float));
}

size_t ntArrayGetF64(const NT_ARRAY *array, const size_t offset, double *value)
{
    return ntArrayGet(array, offset, value, sizeof(double));
}

size_t ntArrayGetI32(const NT_ARRAY *array, const size_t offset, int32_t *value)
{
    return ntArrayGet(array, offset, value, sizeof(int32_t));
}

size_t ntArrayGetU32(const NT_ARRAY *array, const size_t offset, uint32_t *value)
{
    return ntArrayGet(array, offset, value, sizeof(uint32_t));
}

size_t ntArrayGetI64(const NT_ARRAY *array, const size_t offset, int64_t *value)
{
    return ntArrayGet(array, offset, value, sizeof(int64_t));
}

size_t ntArrayGetU64(const NT_ARRAY *array, const size_t offset, uint64_t *value)
{
    return ntArrayGet(array, offset, value, sizeof(uint64_t));
}

bool ntArrayFind(const NT_ARRAY *array, const void *data, const size_t dataSize, size_t *offset)
{
    if (array->count < dataSize)
        return false;

    const uint8_t *pArray = (uint8_t *)array->data;
    const uint8_t *pData = (uint8_t *)data;

    const uint8_t *pArrayMax = pArray + (array->count - dataSize);
    const uint8_t *pDataMax = pData + dataSize;

    for (const uint8_t *i = pArray; i <= pArrayMax; ++i)
    {
        bool find = true;
        for (const uint8_t *j = pData, *k = i; j < pDataMax && k <= pArrayMax + dataSize; ++j, ++k)
        {
            if (*k != *j)
            {
                find = false;
                break;
            }
        }

        if (find)
        {
            *offset = i - pArray;
            return true;
        }
    }
    return false;
}
