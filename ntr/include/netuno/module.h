#ifndef NETUNO_TYPE_H
#define NETUNO_TYPE_H

#include "common.h"
#include "symbol.h"

typedef struct _NT_MODULE
{
    const char_t *name;
    const size_t nameLen;
    const NT_SYMBOL_TABLE *symbols;
} NT_MODULE;

#endif
