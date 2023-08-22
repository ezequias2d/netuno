#include "netuno/nil/basic_block.h"
#include "pargument.h"
#include <netuno/memory.h>
#include <netuno/nil/context.h>
#include <netuno/nil/type.h>

NIL_VALUE *ntCreateArgument(NIL_TYPE *valueType, const char_t *name,
                            NIL_FUNCTION *function, size_t argIndex)
{
    NT_STRING *id = nilGetPrefixedId(nilGetTypeContext(valueType), name);

    NIL_ARGUMENT *arg = (NIL_ARGUMENT *)ntMalloc(sizeof(NIL_ARGUMENT));
    arg->value.name = id;
    arg->value.valueType = NIL_VALUE_TYPE_CONSTANT;
    arg->value.type = valueType;
    arg->value.dbgLoc = NULL;

    arg->function = function;
    arg->argIndex = argIndex;

    return (NIL_VALUE *)arg;
}
