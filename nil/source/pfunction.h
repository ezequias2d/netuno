#ifndef NT_PFUNCTION_H
#define NT_PFUNCTION_H

#include "netuno/nil/basic_block.h"
#include "netuno/string.h"
#include "pargument.h"
#include "plist.h"
#include "ptype.h"
#include <netuno/nil/function.h>
#include <netuno/nil/type.h>

struct _NIL_FUNCTION
{
    NT_STRING *name;
    NIL_FUNCTION_TYPE *functionType;

    size_t argumentCount;
    NIL_ARGUMENT *args;

    LIST(NIL_BASIC_BLOCK *, blocks, list);
};

#endif
