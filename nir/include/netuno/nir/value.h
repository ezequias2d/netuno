#ifndef NIR_VALUE_H
#define NIR_VALUE_H

#include <netuno/common.h>
#include <netuno/nir/debug.h>
#include <netuno/string.h>

NT_HANDLE(NIR_VALUE)
NT_HANDLE(NIR_TYPE)

typedef uint32_t enum_t;
#define NIR_VALUE_UNDEF ((NIR_VALUE *)(-1))

/**
 * @ingroup Value
 * @brief This enum identifier type of value.
 * @typedef NIR_VALUE_TYPE
 * @enum _NIR_VALUE_TYPE
 * @see @ref NIR_VALUE
 */
NT_ENUM(NIR_VALUE_TYPE){
    NIR_VALUE_TYPE_ARGUMENT,
    NIR_VALUE_TYPE_CONSTANT,
    NIR_VALUE_TYPE_INSTRUCTION,
};

const NT_STRING *nirGetValueName(NIR_VALUE *value);
const NIR_DEBUG_LOC *nirGetValueDebugLoc(NIR_VALUE *value);
bool nirIsValueType(NIR_VALUE *value, NIR_VALUE_TYPE type);
NIR_TYPE *nirGetType(NIR_VALUE *value);
void nirPrintValue(NIR_VALUE *value);
void nirPrintValueName(NIR_VALUE *value);
#endif
