#ifndef NIR_PCONSTANT_H
#define NIR_PCONSTANT_H

#include "pvalue.h"
#include <netuno/nir/constant.h>
NT_HANDLE(NIR_TYPE)
NT_HANDLE(NIR_CONSTANT)

/**
 * @ingroup Value
 * @struct _NIR_CONSTANT
 * @typedef NIR_CONSTANT
 * @brief This type represents a constant.
 * @see @ref NIR_VALUE
 */
struct _NIR_CONSTANT
{
    /// @brief Base value.
    NIR_VALUE value;
    /// @brief Size of data field.
    size_t numBytes;
    /// @brief Constant data.
    uint8_t data[];
};

#endif
