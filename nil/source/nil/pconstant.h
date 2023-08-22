#ifndef NIL_PCONSTANT_H
#define NIL_PCONSTANT_H

#include "pvalue.h"
#include <netuno/nil/constant.h>
NT_HANDLE(NIL_TYPE)
NT_HANDLE(NIL_CONSTANT)

/**
 * @ingroup Value
 * @struct _NIL_CONSTANT
 * @typedef NIL_CONSTANT
 * @brief This type represents a constant.
 * @see @ref NIL_VALUE
 */
struct _NIL_CONSTANT
{
    /// @brief Base value.
    NIL_VALUE value;
    /// @brief Size of data field.
    size_t numBytes;
    /// @brief Contanst contains string.
    bool string;
    /// @brief Constant data.
    uint8_t data[];
};

#endif
