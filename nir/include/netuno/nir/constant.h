#ifndef NIR_CONSTANT_H
#define NIR_CONSTANT_H

#include "netuno/nir/context.h"
#include <netuno/common.h>

NT_HANDLE(NIR_VALUE)
NT_HANDLE(NIR_TYPE)

/***********/
/* Integer */
/***********/
NIR_VALUE *nirGetIntTrue(NIR_TYPE *type);
NIR_VALUE *nirGetIntFalse(NIR_TYPE *type);
NIR_VALUE *nirGetIntBool(NIR_TYPE *type, bool value);
NIR_VALUE *nirGetInt(NIR_TYPE *type, uint64_t value, bool isSigned);
NIR_VALUE *nirGetIntAllOnes(NIR_TYPE *type);
bool nirIsIntValueValid(NIR_TYPE *type, uint64_t value);

/*********/
/* Float */
/*********/
NIR_VALUE *nirGetFloat(NIR_TYPE *type, double value);
NIR_VALUE *nirGetFloatNaN(NIR_TYPE *type, bool negative);
NIR_VALUE *nirGetFloatInfinity(NIR_TYPE *type, bool negative);
NIR_VALUE *nirGetFloatZero(NIR_TYPE *type, bool negative);
bool nirIsFloatValueValid(NIR_TYPE *type, double value);

/****************/
/* Array/Struct */
/****************/
NIR_VALUE *nirGetConstantAggregate(NIR_TYPE *arrayType, size_t valueCount,
                                   NIR_VALUE **values);

/*****************/
/* Constant Data */
/*****************/
NIR_VALUE *nirGetConstantData(NIR_TYPE *elementType, size_t elementCount,
                              void *data);
#endif
