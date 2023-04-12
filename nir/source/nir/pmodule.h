#ifndef NIR_PMODULE_H
#define NIR_PMODULE_H

#include "netuno/nir/instruction.h"
#include "netuno/nir/type.h"
#include "plist.h"
#include <netuno/nir/module.h>

struct _NIR_MODULE
{
    NT_STRING *name;
    NT_STRING *sourceFileName;

    LIST(NIR_FUNCTION *, functions, list);
};

#endif
