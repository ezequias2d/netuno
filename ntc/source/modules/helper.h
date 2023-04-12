#ifndef NT_MODULES_HELPER_H
#define NT_MODULES_HELPER_H

#include "netuno/nir/basic_block.h"
#include "netuno/nir/context.h"
#include "netuno/nir/instruction.h"
#include "netuno/nir/module.h"
#include "scope.h"
#include "type.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <stdio.h>

NIR_TYPE *toNirType(NIR_CONTEXT *c, const NT_TYPE *type);

static void addFunction(NIR_CONTEXT *context, NT_TYPE *type, const char_t *name,
                        NT_SYMBOL_TYPE symbolType, const NT_TYPE *delegateType, NIR_MODULE *module)
{
    NT_STRING *nameStr = ntCopyString(name, ntStrLen(name));
    NIR_TYPE *functionType = toNirType(context, delegateType);
    NIR_FUNCTION *function = nirGetOrInsertFunction(module, name, functionType);

    const NT_SYMBOL entry = {
        .symbol_name = nameStr,
        .type = symbolType | SYMBOL_TYPE_PUBLIC,
        .exprType = delegateType,
        .weak = false,
        .value = (NIR_VALUE *)function,
    };
    const bool result = ntInsertSymbol(&type->fields, &entry);
    assert(result);
}
#endif
