#ifndef NIR_PVALUE_H
#define NIR_PVALUE_H

#include <netuno/nir/value.h>

/**
 * @ingroup Value
 * @typedef NIR_VALUE
 * @struct _NIR_VALUE
 * @brief This type represents a value/single assigned register.
 * @see @ref NIR_VALUE_TYPE
 */
struct _NIR_VALUE
{
    /// @brief Symbol table entry to identify the value.
    NT_STRING *name;
    /// @brief Type of value.
    NIR_VALUE_TYPE valueType;
    /// @brief Debug information about code that generate this value.
    NIR_DEBUG_LOC *dbgLoc;
    NIR_TYPE *type;
};

#endif
