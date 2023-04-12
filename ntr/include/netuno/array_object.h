#ifndef NT_ARRAY_OBJECT_H
#define NT_ARRAY_OBJECT_H

#include <netuno/array.h>
#include <netuno/object.h>

typedef struct _NT_ARRAY_OBJECT NT_ARRAY_OBJECT;

struct _NT_ARRAY_OBJECT
{
    NT_OBJECT object;
    NT_ARRAY array;
};

const NT_TYPE *ntArrayType(const NT_TYPE *t);

#endif
