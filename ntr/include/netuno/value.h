#ifndef NETUNO_VALUE_H
#define NETUNO_VALUE_H

#include "common.h"

typedef struct
{
    union {
        double DOUBLE;
        float FLOAT;
        uint32_t I32;
        uint32_t U32;
        uint32_t I64;
        uint64_t U64;
    };
} NT_VALUE;
#endif
