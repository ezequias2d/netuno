#include "netuno/nir/basic_block.h"
#include "pargument.h"
#include <netuno/memory.h>
#include <netuno/nir/context.h>
#include <netuno/nir/type.h>

NIR_VALUE *ntCreateArgument(NIR_TYPE *valueType, const char_t *name,
                            NIR_FUNCTION *function, size_t argIndex)
{
    NT_STRING *id = nirGetPrefixedId(nirGetTypeContext(valueType), name);

    NIR_ARGUMENT *arg = (NIR_ARGUMENT *)ntMalloc(sizeof(NIR_ARGUMENT));
    arg->value.name = id;
    arg->value.valueType = NIR_VALUE_TYPE_CONSTANT;
    arg->value.type = valueType;
    arg->value.dbgLoc = NULL;

    arg->function = function;
    arg->argIndex = argIndex;

    return (NIR_VALUE *)arg;
}
