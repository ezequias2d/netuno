#ifndef NIL_VALUE_H
#define NIL_VALUE_H

#include <netuno/common.h>
#include <netuno/nil/debug.h>
#include <netuno/string.h>

NT_HANDLE(NIL_VALUE)
NT_HANDLE(NIL_TYPE)

typedef uint32_t enum_t;
#define NIL_VALUE_UNDEF ((NIL_VALUE *)(-1))

/**
 * @ingroup Value
 * @brief This enum identifier type of value.
 * @typedef NIL_VALUE_TYPE
 * @enum _NIL_VALUE_TYPE
 * @see @ref NIL_VALUE
 */
NT_ENUM(NIL_VALUE_TYPE){
    NIL_VALUE_TYPE_ARGUMENT,
    NIL_VALUE_TYPE_CONSTANT,
    NIL_VALUE_TYPE_INSTRUCTION,
};

const NT_STRING *nilGetValueName(NIL_VALUE *value);
const NIL_DEBUG_LOC *nilGetValueDebugLoc(NIL_VALUE *value);
bool nilIsValueType(NIL_VALUE *value, NIL_VALUE_TYPE type);
NIL_TYPE *nilGetType(NIL_VALUE *value);
void nilPrintValue(NIL_VALUE *value);
void nilPrintValueName(NIL_VALUE *value);
#endif
