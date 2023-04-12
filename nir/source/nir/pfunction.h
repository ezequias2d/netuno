#ifndef NT_PFUNCTION_H
#define NT_PFUNCTION_H

#include "netuno/nir/basic_block.h"
#include "netuno/string.h"
#include "nir/pargument.h"
#include "plist.h"
#include "ptype.h"
#include <netuno/nir/function.h>
#include <netuno/nir/type.h>

struct _NIR_FUNCTION
{
    NT_STRING *name;
    NIR_FUNCTION_TYPE *functionType;

    size_t argumentCount;
    NIR_ARGUMENT *args;

    LIST(NIR_BASIC_BLOCK *, blocks, list);
};

#endif
