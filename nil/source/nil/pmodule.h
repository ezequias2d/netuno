#ifndef NIL_PMODULE_H
#define NIL_PMODULE_H

#include "netuno/nil/instruction.h"
#include "netuno/nil/type.h"
#include "plist.h"
#include <netuno/nil/module.h>

struct _NIL_MODULE
{
    NT_STRING *name;
    NT_STRING *sourceFileName;

    LIST(NIL_FUNCTION *, functions, list);
};

#endif
