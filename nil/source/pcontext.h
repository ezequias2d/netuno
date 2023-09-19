#ifndef NIL_PCONTEXT_H
#define NIL_PCONTEXT_H

#include "netuno/common.h"
#include <netuno/array.h>
#include <netuno/nil/context.h>
#include <netuno/string.h>
#include <netuno/table.h>

NT_HANDLE(NIL_TYPE)
NT_HANDLE(NIL_POINTER_TYPE)

struct _NIL_CONTEXT
{
    NIL_TYPE *errorType;
    NIL_TYPE *voidType;
    NIL_TYPE *labelType;
    NIL_TYPE *floatType;
    NIL_TYPE *doubleType;
    NIL_POINTER_TYPE *opaquePtrType;

    NT_ARRAY integerTypes;
    NT_ARRAY functionTypes;
    NT_ARRAY arrayTypes;
    NT_ARRAY ptrTypes;
    NT_ARRAY structTypes;

    NT_TABLE *prefixes;
};

#endif
