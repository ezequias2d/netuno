#include "netuno/nir/value.h"
#include "pconstant.h"
#include "pcontext.h"
#include <assert.h>
#include <math.h>
#include <netuno/memory.h>
#include <netuno/nir/type.h>
#include <netuno/string.h>
#include <stdint.h>

static NIR_VALUE *nirGetConstant(NIR_TYPE *valueType, size_t size,
                                 const void *data)
{
    assert(valueType);
    NT_STRING *id = nirGetPrefixedId(nirGetTypeContext(valueType), U"const");

    NIR_CONSTANT *constant =
        (NIR_CONSTANT *)ntMalloc(sizeof(NIR_CONSTANT) + size);

    constant->value.name = id;
    constant->value.valueType = NIR_VALUE_TYPE_CONSTANT;
    constant->value.dbgLoc = NULL;
    constant->value.type = valueType;

    constant->numBytes = size;
    ntMemcpy(constant->data, data, size);

    return (NIR_VALUE *)constant;
}

NIR_VALUE *nirGetIntTrue(NIR_TYPE *valueType)
{
    assert(valueType);
    return nirGetIntBool(valueType, true);
}

NIR_VALUE *nirGetIntFalse(NIR_TYPE *valueType)
{
    assert(valueType);
    return nirGetIntBool(valueType, false);
}

NIR_VALUE *nirGetIntBool(NIR_TYPE *valueType, bool value)
{
    assert(valueType);
    return nirGetInt(valueType, value, false);
}

static uint64_t extend(uint64_t value, size_t bit)
{
    const uint64_t m = UINT64_C(1) << (bit - 1);
    return (value ^ m) - m;
}

NIR_VALUE *nirGetInt(NIR_TYPE *valueType, uint64_t value, bool isSigned)
{
    assert(nirIsIntegerType(valueType));

    if (isSigned)
    {
        const size_t bits = nirGetIntegerBitWidth(valueType);
        value = extend(value, bits);
    }

    return nirGetConstant(valueType, sizeof(uint64_t), &value);
}

NIR_VALUE *nirGetIntAllOnes(NIR_TYPE *valueType)
{
    assert(nirIsIntegerType(valueType));
    const uint64_t value = (1 << nirGetIntegerBitWidth(valueType)) - 1;
    return nirGetInt(valueType, value, false);
}

bool nirIsIntValueValid(NIR_TYPE *type, uint64_t value)
{
    if (!nirIsIntegerType(type))
        return false;

    const size_t numBits = nirGetIntegerBitWidth(type);

    if (numBits == 1)
        return value == 0 || value == 1;
    return numBits >= 64 || value <= (UINT64_MAX >> (64 - numBits));
}

NIR_VALUE *nirGetFloat(NIR_TYPE *type, double value)
{
    return nirGetConstant(type, sizeof(double), &value);
}

NIR_VALUE *nirGetFloatNaN(NIR_TYPE *type, bool negative)
{
    const double value = negative ? -NAN : NAN;
    return nirGetFloat(type, value);
}

NIR_VALUE *nirGetFloatInfinity(NIR_TYPE *type, bool negative)
{
    const double value = negative ? -INFINITY : INFINITY;
    return nirGetFloat(type, value);
}

NIR_VALUE *nirGetFloatZero(NIR_TYPE *type, bool negative)
{
    const double value = negative ? -0.0 : 0.0;
    return nirGetFloat(type, value);
}

bool nirIsFloatValueValid(NIR_TYPE *type, double value)
{
    switch (nirGetTypeID(type))
    {
    case NIR_TYPE_FLOAT:
        return (double)(float)value == value;
    case NIR_TYPE_DOUBLE:
        return true;
    default:
        return false;
    }
}

NIR_VALUE *nirGetConstantAggregate(NIR_TYPE *arrayType, size_t valueCount,
                                   NIR_VALUE **values)
{
    assert(nirIsArrayType(arrayType));
    return nirGetConstant(arrayType, sizeof(NIR_VALUE *) * valueCount, values);
}

NIR_VALUE *nirGetConstantData(NIR_TYPE *elementType, size_t elementCount,
                              void *data)
{
    assert(nirIsSized(elementType));
    return nirGetConstant(elementType, elementCount, data);
}
