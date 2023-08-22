#ifndef NIL_PVALUE_H
#define NIL_PVALUE_H

#include <netuno/nil/value.h>

/**
 * @ingroup Value
 * @typedef NIL_VALUE
 * @struct _NIL_VALUE
 * @brief This type represents a value/single assigned register.
 * @see @ref NIL_VALUE_TYPE
 */
struct _NIL_VALUE
{
    /// @brief Symbol table entry to identify the value.
    NT_STRING *name;
    /// @brief Type of value.
    NIL_VALUE_TYPE valueType;
    /// @brief Debug information about code that generate this value.
    NIL_DEBUG_LOC *dbgLoc;
    NIL_TYPE *type;
};

#endif
