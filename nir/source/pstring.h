#ifndef NT_PSTRING_H
#define NT_PSTRING_H

#include <netuno/common.h>

struct _NT_STRING
{
    size_t refCount;
    size_t length;
    char_t *chars;
    uint32_t hash;
};

#endif
