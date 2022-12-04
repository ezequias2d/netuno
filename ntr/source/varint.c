#include <assert.h>
#include <netuno/varint.h>
#include <stdbool.h>
#include <stdio.h>

uint64_t ZigZagEncoding(int64_t value)
{
    return (uint64_t)((value >> 31) ^ (value << 1));
}

int64_t ZigZagDecoding(uint64_t n)
{
    return (int64_t)((n >> 1) ^ (-(n & 1)));
}

size_t ntEncodeVarint(void *dst, const size_t srcSize, const uint64_t value)
{
    if (srcSize == 0)
        return 0;

    uint64_t n = value;
    size_t i = 0;

    do
    {
        uint8_t coded = (uint8_t)(n & 0x7F);
        n >>= 7;
        if (n > 0)
            coded |= 0x80;
        ((uint8_t *)dst)[i++] = coded;
    } while (n != 0 && i < srcSize);
    if (n != 0 && i >= srcSize)
        return 0;

    return i;
}

size_t ntDecodeVarint(const void *src, const size_t srcSize, uint64_t *value)
{
    uint64_t result = 0;
    bool moreBytes = false;
    uint8_t bits = 0;
    uint8_t count = 0;
    do
    {
        assert(count < srcSize);
        const uint8_t readed = ((uint8_t *)src)[count];
        result |= readed << bits;
        moreBytes = (readed & 0x80) != 0;
        bits += 7;
        count++;
    } while (moreBytes);
    *value = result;
    return count;
}
