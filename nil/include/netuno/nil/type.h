#ifndef NIL_TYPE_H
#define NIL_TYPE_H

#include <netuno/array.h>
#include <netuno/common.h>
#include <netuno/nil/context.h>

NT_HANDLE(NIL_TYPE)
NT_HANDLE(NT_STRING)

NT_ENUM(NIL_TYPE_ID){
    NIL_ERROR_TYPE,

    // primitive types
    NIL_TYPE_FLOAT,
    NIL_TYPE_DOUBLE,
    NIL_TYPE_VOID,
    NIL_TYPE_LABEL,

    // derived types
    NIL_TYPE_INTEGER,
    NIL_TYPE_FUNCTION,
    NIL_TYPE_POINTER,
    NIL_TYPE_STRUCT,
    NIL_TYPE_ARRAY,
};

NT_ENUM(NIL_INTEGER_BITS){
    NIL_INTEGER_MIN_BITS = 1,
    NIL_INTEGER_MAX_BITS = (1 << (sizeof(enum_t) - 1)),
};

NIL_TYPE *nilGetErrorType(NIL_CONTEXT *c);
NIL_TYPE *nilGetVoidType(NIL_CONTEXT *c);
NIL_TYPE *nilGetLabelType(NIL_CONTEXT *c);
NIL_TYPE *nilGetFloatType(NIL_CONTEXT *c);
NIL_TYPE *nilGetDoubleType(NIL_CONTEXT *c);
NIL_TYPE *nilGetIntegerType(NIL_CONTEXT *c, NIL_INTEGER_BITS numBits);
NIL_TYPE *nilGetInt1Type(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt8Type(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt16Type(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt32Type(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt64Type(NIL_CONTEXT *c);
NIL_TYPE *nilGetFunctionType(NIL_CONTEXT *c, NIL_TYPE *result,
                             size_t paramCount, NIL_TYPE *const *params,
                             bool isVarArg);
NIL_TYPE *nilGetOpaquePointerType(NIL_CONTEXT *c);

NIL_TYPE *nilGetArrayType(NIL_CONTEXT *c, NIL_TYPE *elementType,
                          uint64_t numElements);
NIL_TYPE *nilGetFloatPtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetDoublePtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetIntegerPtrType(NIL_CONTEXT *c, NIL_INTEGER_BITS n);
NIL_TYPE *nilGetInt1PtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt8PtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt16PtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt32PtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetInt64PtrType(NIL_CONTEXT *c);
NIL_TYPE *nilGetStructType(NIL_CONTEXT *c, size_t elementCount,
                           NIL_TYPE *const *elementTypes);

NIL_CONTEXT *nilGetTypeContext(NIL_TYPE *type);
NIL_TYPE_ID nilGetTypeID(NIL_TYPE *type);
bool nilIsVoidType(NIL_TYPE *type);
bool nilIsLabelType(NIL_TYPE *type);
bool nilIsFloatType(NIL_TYPE *type);
bool nilIsDoubleType(NIL_TYPE *type);
bool nilIsIntegerType(NIL_TYPE *type);
bool nilIsIntegerNType(NIL_TYPE *type, enum_t n);
bool nilIsFunctionType(NIL_TYPE *type);
bool nilIsStructType(NIL_TYPE *type);
bool nilIsArrayType(NIL_TYPE *type);
bool nilIsPointerType(NIL_TYPE *type);
bool nilIsFirstClassType(NIL_TYPE *type);
bool nilIsSingleValueType(NIL_TYPE *type);
bool nilIsAggregateType(NIL_TYPE *type);
bool nilIsSized(NIL_TYPE *type);
size_t nilGetPrimitiveSizeInBits(NIL_TYPE *type);
NIL_INTEGER_BITS nilGetIntegerBitWidth(NIL_TYPE *type);
size_t nilGetFunctionNumParams(NIL_TYPE *type);
NIL_TYPE *nilGetFunctionParamType(NIL_TYPE *type, size_t i);
bool nilIsFunctionVarArg(NIL_TYPE *type);
// NT_STRING *nilGetStructName(NIL_TYPE *type);
size_t nilGetStructNumElements(NIL_TYPE *type);
NIL_TYPE *nilGetStructElementType(NIL_TYPE *type, size_t n);
size_t nilGetArrayNumElements(NIL_TYPE *type);
NIL_TYPE *nilGetArrayElementType(NIL_TYPE *type);
NIL_TYPE *nilGetPointerTo(NIL_TYPE *type);
NIL_TYPE *nilGetPointeeType(NIL_TYPE *ptrType);
bool nilIsValidElementType(NIL_TYPE *elementType);
bool nilIsValidReturnType(NIL_TYPE *type);
bool nilIsValidArgumentType(NIL_TYPE *type);
bool nilIsOpaque(NIL_TYPE *type);

bool nilSerializeType(NIL_TYPE *type, NT_ARRAY *array);
void nilPrintType(NIL_TYPE *type);
#endif
