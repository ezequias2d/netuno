#include "netuno/nil/value.h"
#include "pconstant.h"
#include "pcontext.h"
#include <assert.h>
#include <math.h>
#include <netuno/memory.h>
#include <netuno/nil/type.h>
#include <netuno/string.h>
#include <stdbool.h>
#include <stdint.h>

static NIL_VALUE *nilGetConstant(NIL_TYPE *valueType, size_t size,
                                 const void *data, bool string)
{
    assert(valueType);
    NT_STRING *id = nilGetPrefixedId(nilGetTypeContext(valueType), U"const");

    NIL_CONSTANT *constant =
        (NIL_CONSTANT *)ntMalloc(sizeof(NIL_CONSTANT) + size);

    constant->value.name = id;
    constant->value.valueType = NIL_VALUE_TYPE_CONSTANT;
    constant->value.dbgLoc = NULL;
    constant->value.type = valueType;
    constant->string = string;

    constant->numBytes = size;
    ntMemcpy(constant->data, data, size);

    return (NIL_VALUE *)constant;
}

NIL_VALUE *nilGetIntTrue(NIL_TYPE *valueType)
{
    assert(valueType);
    return nilGetIntBool(valueType, true);
}

NIL_VALUE *nilGetIntFalse(NIL_TYPE *valueType)
{
    assert(valueType);
    return nilGetIntBool(valueType, false);
}

NIL_VALUE *nilGetIntBool(NIL_TYPE *valueType, bool value)
{
    assert(valueType);
    return nilGetInt(valueType, value, false);
}

static uint64_t extend(uint64_t value, size_t bit)
{
    const uint64_t m = UINT64_C(1) << (bit - 1);
    return (value ^ m) - m;
}

NIL_VALUE *nilGetInt(NIL_TYPE *valueType, uint64_t value, bool isSigned)
{
    assert(nilIsIntegerType(valueType));

    if (isSigned)
    {
        const size_t bits = nilGetIntegerBitWidth(valueType);
        value = extend(value, bits);
    }

    return nilGetConstant(valueType, sizeof(uint64_t), &value, false);
}

NIL_VALUE *nilGetIntAllOnes(NIL_TYPE *valueType)
{
    assert(nilIsIntegerType(valueType));
    const uint64_t value = (1 << nilGetIntegerBitWidth(valueType)) - 1;
    return nilGetInt(valueType, value, false);
}

bool nilIsIntValueValid(NIL_TYPE *type, uint64_t value)
{
    if (!nilIsIntegerType(type))
        return false;

    const size_t numBits = nilGetIntegerBitWidth(type);

    if (numBits == 1)
        return value == 0 || value == 1;
    return numBits >= 64 || value <= (UINT64_MAX >> (64 - numBits));
}

NIL_VALUE *nilGetFloat(NIL_TYPE *type, double value)
{
    return nilGetConstant(type, sizeof(double), &value, false);
}

NIL_VALUE *nilGetFloatNaN(NIL_TYPE *type, bool negative)
{
    const double value = negative ? -NAN : NAN;
    return nilGetFloat(type, value);
}

NIL_VALUE *nilGetFloatInfinity(NIL_TYPE *type, bool negative)
{
    const double value = negative ? -INFINITY : INFINITY;
    return nilGetFloat(type, value);
}

NIL_VALUE *nilGetFloatZero(NIL_TYPE *type, bool negative)
{
    const double value = negative ? -0.0 : 0.0;
    return nilGetFloat(type, value);
}

bool nilIsFloatValueValid(NIL_TYPE *type, double value)
{
    switch (nilGetTypeID(type))
    {
    case NIL_TYPE_FLOAT:
        return (double)(float)value == value;
    case NIL_TYPE_DOUBLE:
        return true;
    default:
        return false;
    }
}

NIL_VALUE *nilGetConstantAggregate(NIL_TYPE *arrayType, size_t valueCount,
                                   NIL_VALUE **values)
{
    assert(nilIsArrayType(arrayType));
    return nilGetConstant(arrayType, sizeof(NIL_VALUE *) * valueCount, values,
                          false);
}

NIL_VALUE *nilGetConstantData(NIL_TYPE *elementType, size_t elementCount,
                              void *data)
{
    assert(nilIsSized(elementType));
    return nilGetConstant(elementType, elementCount, data, false);
}

NIL_VALUE *nilGetConstantStringData(NIL_TYPE *elementType, size_t elementCount,
                                    void *data)
{
    assert(nilIsSized(elementType));
    return nilGetConstant(elementType,
                          elementCount * nilGetIntegerBitWidth(elementType),
                          data, true);
}
