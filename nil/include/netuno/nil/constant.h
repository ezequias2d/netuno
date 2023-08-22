#ifndef NIL_CONSTANT_H
#define NIL_CONSTANT_H

#include <netuno/nil/context.h>
#include <netuno/common.h>

NT_HANDLE(NIL_VALUE)
NT_HANDLE(NIL_TYPE)

/***********/
/* Integer */
/***********/
NIL_VALUE *nilGetIntTrue(NIL_TYPE *type);
NIL_VALUE *nilGetIntFalse(NIL_TYPE *type);
NIL_VALUE *nilGetIntBool(NIL_TYPE *type, bool value);
NIL_VALUE *nilGetInt(NIL_TYPE *type, uint64_t value, bool isSigned);
NIL_VALUE *nilGetIntAllOnes(NIL_TYPE *type);
bool nilIsIntValueValid(NIL_TYPE *type, uint64_t value);

/*********/
/* Float */
/*********/
NIL_VALUE *nilGetFloat(NIL_TYPE *type, double value);
NIL_VALUE *nilGetFloatNaN(NIL_TYPE *type, bool negative);
NIL_VALUE *nilGetFloatInfinity(NIL_TYPE *type, bool negative);
NIL_VALUE *nilGetFloatZero(NIL_TYPE *type, bool negative);
bool nilIsFloatValueValid(NIL_TYPE *type, double value);

/****************/
/* Array/Struct */
/****************/
NIL_VALUE *nilGetConstantAggregate(NIL_TYPE *arrayType, size_t valueCount,
                                   NIL_VALUE **values);

/*****************/
/* Constant Data */
/*****************/
NIL_VALUE *nilGetConstantData(NIL_TYPE *elementType, size_t elementCount,
                              void *data);
NIL_VALUE *nilGetConstantStringData(NIL_TYPE *elementType, size_t elementCount,
                                    void *data);
#endif
