#ifndef NIR_PCONTEXT_H
#define NIR_PCONTEXT_H

#include "netuno/common.h"
#include <netuno/array.h>
#include <netuno/nir/context.h>
#include <netuno/string.h>
#include <netuno/table.h>

NT_HANDLE(NIR_TYPE)
NT_HANDLE(NIR_POINTER_TYPE)

struct _NIR_CONTEXT
{
    NIR_TYPE *errorType;
    NIR_TYPE *voidType;
    NIR_TYPE *labelType;
    NIR_TYPE *floatType;
    NIR_TYPE *doubleType;
    NIR_POINTER_TYPE *opaquePtrType;

    NT_ARRAY integerTypes;
    NT_ARRAY functionTypes;
    NT_ARRAY arrayTypes;
    NT_ARRAY ptrTypes;
    NT_ARRAY structTypes;

    NT_TABLE *prefixes;
};

#endif
