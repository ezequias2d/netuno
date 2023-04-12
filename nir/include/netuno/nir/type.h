#ifndef NIR_TYPE_H
#define NIR_TYPE_H

#include <netuno/common.h>
#include <netuno/nir/context.h>

NT_HANDLE(NIR_TYPE)
NT_HANDLE(NT_STRING)

NT_ENUM(NIR_TYPE_ID){
    NIR_ERROR_TYPE,

    // primitive types
    NIR_TYPE_FLOAT,
    NIR_TYPE_DOUBLE,
    NIR_TYPE_VOID,
    NIR_TYPE_LABEL,

    // derived types
    NIR_TYPE_INTEGER,
    NIR_TYPE_FUNCTION,
    NIR_TYPE_POINTER,
    NIR_TYPE_STRUCT,
    NIR_TYPE_ARRAY,
};

NT_ENUM(NIR_INTEGER_BITS){
    NIR_INTEGER_MIN_BITS = 1,
    NIR_INTEGER_MAX_BITS = (1 << (sizeof(enum_t) - 1)),
};

NIR_TYPE *nirGetErrorType(NIR_CONTEXT *c);
NIR_TYPE *nirGetVoidType(NIR_CONTEXT *c);
NIR_TYPE *nirGetLabelType(NIR_CONTEXT *c);
NIR_TYPE *nirGetFloatType(NIR_CONTEXT *c);
NIR_TYPE *nirGetDoubleType(NIR_CONTEXT *c);
NIR_TYPE *nirGetIntegerType(NIR_CONTEXT *c, NIR_INTEGER_BITS numBits);
NIR_TYPE *nirGetInt1Type(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt8Type(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt16Type(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt32Type(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt64Type(NIR_CONTEXT *c);
NIR_TYPE *nirGetFunctionType(NIR_CONTEXT *c, NIR_TYPE *result,
                             size_t paramCount, NIR_TYPE *const *params,
                             bool isVarArg);
NIR_TYPE *nirGetOpaquePointerType(NIR_CONTEXT *c);

NIR_TYPE *nirGetArrayType(NIR_CONTEXT *c, NIR_TYPE *elementType,
                          uint64_t numElements);
NIR_TYPE *nirGetFloatPtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetDoublePtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetIntegerPtrType(NIR_CONTEXT *c, NIR_INTEGER_BITS n);
NIR_TYPE *nirGetInt1PtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt8PtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt16PtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt32PtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetInt64PtrType(NIR_CONTEXT *c);
NIR_TYPE *nirGetStructType(NIR_CONTEXT *c, size_t elementCount,
                           NIR_TYPE *const *elementTypes);

NIR_CONTEXT *nirGetTypeContext(NIR_TYPE *type);
NIR_TYPE_ID nirGetTypeID(NIR_TYPE *type);
bool nirIsVoidType(NIR_TYPE *type);
bool nirIsLabelType(NIR_TYPE *type);
bool nirIsFloatType(NIR_TYPE *type);
bool nirIsDoubleType(NIR_TYPE *type);
bool nirIsIntegerType(NIR_TYPE *type);
bool nirIsIntegerNType(NIR_TYPE *type, enum_t n);
bool nirIsFunctionType(NIR_TYPE *type);
bool nirIsStructType(NIR_TYPE *type);
bool nirIsArrayType(NIR_TYPE *type);
bool nirIsPointerType(NIR_TYPE *type);
bool nirIsFirstClassType(NIR_TYPE *type);
bool nirIsSingleValueType(NIR_TYPE *type);
bool nirIsAggregateType(NIR_TYPE *type);
bool nirIsSized(NIR_TYPE *type);
size_t nirGetPrimitiveSizeInBits(NIR_TYPE *type);
NIR_INTEGER_BITS nirGetIntegerBitWidth(NIR_TYPE *type);
size_t nirGetFunctionNumParams(NIR_TYPE *type);
NIR_TYPE *nirGetFunctionParamType(NIR_TYPE *type, size_t i);
bool nirIsFunctionVarArg(NIR_TYPE *type);
// NT_STRING *nirGetStructName(NIR_TYPE *type);
size_t nirGetStructNumElements(NIR_TYPE *type);
NIR_TYPE *nirGetStructElementType(NIR_TYPE *type, size_t n);
size_t nirGetArrayNumElements(NIR_TYPE *type);
NIR_TYPE *nirGetArrayElementType(NIR_TYPE *type);
NIR_TYPE *nirGetPointerTo(NIR_TYPE *type);
NIR_TYPE *nirGetPointeeType(NIR_TYPE *ptrType);
bool nirIsValidElementType(NIR_TYPE *elementType);
bool nirIsValidReturnType(NIR_TYPE *type);
bool nirIsValidArgumentType(NIR_TYPE *type);
bool nirIsOpaque(NIR_TYPE *type);

void nirPrintType(NIR_TYPE *type);
#endif
