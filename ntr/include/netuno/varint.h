#ifndef NETUNO_VARINT_H
#define NETUNO_VARINT_H

#include "common.h"

size_t ntVarintEncodedSize(uint64_t n);
uint64_t ZigZagEncoding(int64_t value);
int64_t ZigZagDecoding(uint64_t value);
size_t ntEncodeVarint(void *dst, const size_t srcSize, const uint64_t value);
size_t ntDecodeVarint(const void *src, const size_t srcSize, uint64_t *value);
#endif
