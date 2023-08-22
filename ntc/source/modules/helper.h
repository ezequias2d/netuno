#ifndef NT_MODULES_HELPER_H
#define NT_MODULES_HELPER_H

#include "netuno/nil/basic_block.h"
#include "netuno/nil/context.h"
#include "netuno/nil/instruction.h"
#include "netuno/nil/module.h"
#include "scope.h"
#include "type.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <stdio.h>

NIL_TYPE *toNirType(NIL_CONTEXT *c, const NT_TYPE *type);

static void addFunction(NIL_CONTEXT *context, NT_TYPE *type, const char_t *name,
                        NT_SYMBOL_TYPE symbolType, const NT_TYPE *delegateType, NIL_MODULE *module)
{
    NT_STRING *nameStr = ntCopyString(name, ntStrLen(name));
    NIL_TYPE *functionType = toNirType(context, delegateType);
    NIL_FUNCTION *function = nilGetOrInsertFunction(module, name, functionType);

    const NT_SYMBOL entry = {
        .symbol_name = nameStr,
        .type = symbolType | SYMBOL_TYPE_PUBLIC,
        .exprType = delegateType,
        .weak = false,
        .value = (NIL_VALUE *)function,
    };
    const bool result = ntInsertSymbol(&type->fields, &entry);
    assert(result);
}
#endif
