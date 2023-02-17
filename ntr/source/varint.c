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

size_t ntEncodeVarint(void *dst, const size_t dstSize, const uint64_t value)
{
    if (dstSize == 0)
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
    } while (n != 0 && i < dstSize);
    if (n != 0 && i >= dstSize)
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
        result |= (readed & 0x7F) << bits;
        moreBytes = (readed & 0x80) != 0;
        bits += 7;
        count++;
    } while (moreBytes);
    if (value)
        *value = result;
    return count;
}

size_t ntVarintEncodedSize(uint64_t n)
{
    const uint64_t s7 = 1 << 7;
    const uint64_t s14 = 1 << 14;
    const uint64_t s21 = 1 << 21;
    const uint64_t s28 = 1 << 28;

    if (n < s7)
        return 1;
    else if (n >= s7 && n < s14)
        return 2;
    else if (n >= s14 && n < s21)
        return 3;
    else if (n >= s21 && n < s28)
        return 4;
    return 5;
}
