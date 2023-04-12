#ifndef NIR_PARGUMENT_H
#define NIR_PARGUMENT_H

#include "netuno/common.h"
#include "netuno/nir/basic_block.h"
#include "pvalue.h"
#include <netuno/nir/argument.h>

NT_HANDLE(NIR_ARGUMENT)
struct _NIR_ARGUMENT
{
    /// @brief Base value.
    NIR_VALUE value;
    NIR_FUNCTION *function;
    size_t argIndex;
};

#endif
