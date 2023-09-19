#ifndef NIL_PARGUMENT_H
#define NIL_PARGUMENT_H

#include "netuno/common.h"
#include "netuno/nil/basic_block.h"
#include "pvalue.h"
#include <netuno/nil/argument.h>

NT_HANDLE(NIL_ARGUMENT)
struct _NIL_ARGUMENT
{
    /// @brief Base value.
    NIL_VALUE value;
    NIL_FUNCTION *function;
    size_t argIndex;
};

#endif
