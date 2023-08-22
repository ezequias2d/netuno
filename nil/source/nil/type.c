#include "netuno/nil/type.h"
#include "colors.h"
#include "netuno/array.h"
#include "netuno/memory.h"
#include "netuno/nil/binary.h"
#include "netuno/nil/context.h"
#include "netuno/string.h"
#include "pcontext.h"
#include "ptype.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

NIL_TYPE *nilGetErrorType(NIL_CONTEXT *c)
{
    return c->errorType;
}

NIL_TYPE *nilGetVoidType(NIL_CONTEXT *c)
{
    return c->voidType;
}

NIL_TYPE *nilGetLabelType(NIL_CONTEXT *c)
{
    return c->labelType;
}

NIL_TYPE *nilGetFloatType(NIL_CONTEXT *c)
{
    return c->floatType;
}

NIL_TYPE *nilGetDoubleType(NIL_CONTEXT *c)
{
    return c->doubleType;
}

NIL_TYPE *nilGetOpaquePointerType(NIL_CONTEXT *c)
{
    assert(c);
    return (NIL_TYPE *)c->opaquePtrType;
}

NIL_TYPE *nilGetIntegerType(NIL_CONTEXT *c, NIL_INTEGER_BITS numBits)
{
    NIL_INTEGER_TYPE *type = NULL;
    for (size_t i = 0; i < c->integerTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->integerTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIL_TYPE_INTEGER);
        if (type->numBits == numBits)
            return (NIL_TYPE *)type;
    }

    type = (NIL_INTEGER_TYPE *)ntMalloc(sizeof(NIL_INTEGER_TYPE));
    type->type.id = NIL_TYPE_INTEGER;
    type->type.context = c;
    type->numBits = numBits;
    ntArrayAdd(&c->integerTypes, &type, sizeof(void *));

    return (NIL_TYPE *)type;
}

NIL_TYPE *nilGetInt1Type(NIL_CONTEXT *c)
{
    return nilGetIntegerType(c, 1);
}

NIL_TYPE *nilGetInt8Type(NIL_CONTEXT *c)
{
    return nilGetIntegerType(c, 8);
}

NIL_TYPE *nilGetInt16Type(NIL_CONTEXT *c)
{
    return nilGetIntegerType(c, 16);
}

NIL_TYPE *nilGetInt32Type(NIL_CONTEXT *c)
{
    return nilGetIntegerType(c, 32);
}

NIL_TYPE *nilGetInt64Type(NIL_CONTEXT *c)
{
    return nilGetIntegerType(c, 64);
}

NIL_TYPE *nilGetFunctionType(NIL_CONTEXT *c, NIL_TYPE *result,
                             size_t paramCount, NIL_TYPE *const *params,
                             bool isVarArg)
{
    NIL_FUNCTION_TYPE *type;
    for (size_t i = 0; i < c->functionTypes.count; i += sizeof(void *))
    {
        const bool getResult = ntArrayGet(&c->functionTypes, i, &type,
                                          sizeof(void *)) == sizeof(void *);
        assert(getResult);
        assert(type->type.id == NIL_TYPE_FUNCTION);
        if (type->result == result && type->isVarArg == isVarArg &&
            type->paramCount == paramCount &&
            memcmp(type->params, params, sizeof(void *) * paramCount) == 0)
            return (NIL_TYPE *)type;
    }

    type = (NIL_FUNCTION_TYPE *)ntMalloc(sizeof(NIL_FUNCTION_TYPE));
    type->type.id = NIL_TYPE_FUNCTION;
    type->type.context = c;
    type->result = result;
    type->paramCount = paramCount;
    type->params = (NIL_TYPE **)ntMalloc(sizeof(void *) * paramCount);
    ntMemcpy(type->params, params, sizeof(void *) * paramCount);
    ntArrayAdd(&c->functionTypes, &type, sizeof(void *));

    return (NIL_TYPE *)type;
}

NIL_TYPE *nilGetArrayType(NIL_CONTEXT *c, NIL_TYPE *elementType,
                          uint64_t numElements)
{
    NIL_ARRAY_TYPE *type;
    for (size_t i = 0; i < c->arrayTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->arrayTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIL_TYPE_ARRAY);
        if (type->numElements == numElements &&
            type->containedType == elementType)
            return (NIL_TYPE *)type;
    }

    type = (NIL_ARRAY_TYPE *)ntMalloc(sizeof(NIL_ARRAY_TYPE));
    type->type.id = NIL_TYPE_ARRAY;
    type->type.context = c;
    type->containedType = elementType;
    type->numElements = numElements;
    ntArrayAdd(&c->arrayTypes, &type, sizeof(void *));

    return (NIL_TYPE *)type;
}

NIL_TYPE *nilGetFloatPtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(c->floatType);
}

NIL_TYPE *nilGetDoublePtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(c->doubleType);
}

NIL_TYPE *nilGetIntegerPtrType(NIL_CONTEXT *c, NIL_INTEGER_BITS n)
{
    return nilGetPointerTo(nilGetIntegerType(c, n));
}

NIL_TYPE *nilGetInt1PtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(nilGetInt1Type(c));
}

NIL_TYPE *nilGetInt8PtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(nilGetInt8Type(c));
}

NIL_TYPE *nilGetInt16PtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(nilGetInt16Type(c));
}

NIL_TYPE *nilGetInt32PtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(nilGetInt32Type(c));
}

NIL_TYPE *nilGetInt64PtrType(NIL_CONTEXT *c)
{
    return nilGetPointerTo(nilGetInt64Type(c));
}

NIL_TYPE *nilGetStructType(NIL_CONTEXT *c, size_t elementCount,
                           NIL_TYPE *const *elementTypes)
{
    NIL_STRUCT_TYPE *type;
    for (size_t i = 0; i < c->arrayTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->structTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIL_TYPE_STRUCT);
        if (type->elementCount == elementCount &&
            memcmp(type->elements, elementTypes,
                   sizeof(NIL_TYPE *) * elementCount) == 0)
            return (NIL_TYPE *)type;
    }

    type = (NIL_STRUCT_TYPE *)ntMalloc(sizeof(NIL_STRUCT_TYPE));
    type->type.id = NIL_TYPE_STRUCT;
    type->type.context = c;
    type->hasBody = true;
    type->isSized = true;
    type->elementCount = elementCount;
    type->elements = (NIL_TYPE **)ntMalloc(sizeof(NIL_TYPE *) * elementCount);
    ntMemcpy(type->elements, elementTypes, sizeof(NIL_TYPE *) * elementCount);
    ntArrayAdd(&c->structTypes, &type, sizeof(void *));

    return (NIL_TYPE *)type;
}

NIL_CONTEXT *nilGetTypeContext(NIL_TYPE *type)
{
    return type->context;
}

NIL_TYPE_ID nilGetTypeID(NIL_TYPE *type)
{
    return type->id;
}

bool nilIsVoidType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_VOID;
}

bool nilIsLabelType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_LABEL;
}

bool nilIsFloatType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_FLOAT;
}

bool nilIsDoubleType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_DOUBLE;
}

bool nilIsIntegerType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_INTEGER;
}

bool nilIsIntegerNType(NIL_TYPE *type, enum_t n)
{
    return type->id == NIL_TYPE_INTEGER &&
           ((NIL_INTEGER_TYPE *)type)->numBits == n;
}

bool nilIsFunctionType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_FUNCTION;
}

bool nilIsStructType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_STRUCT;
}

bool nilIsArrayType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_ARRAY;
}

bool nilIsPointerType(NIL_TYPE *type)
{
    return type->id == NIL_TYPE_POINTER;
}

bool nilIsFirstClassType(NIL_TYPE *type)
{
    return type->id != NIL_TYPE_VOID && type->id != NIL_TYPE_FUNCTION;
}

bool nilIsSingleValueType(NIL_TYPE *type)
{
    return nilIsFloatType(type) || nilIsIntegerType(type) ||
           nilIsDoubleType(type) || nilIsPointerType(type);
}

bool nilIsAggregateType(NIL_TYPE *type)
{
    return nilIsStructType(type) || nilIsArrayType(type);
}

bool nilIsSized(NIL_TYPE *type)
{
    if (nilIsIntegerType(type) || nilIsFloatType(type) ||
        nilIsDoubleType(type) || nilIsPointerType(type))
        return true;

    if (!nilIsStructType(type) && !nilIsArrayType(type))
        return false;

    if (nilIsArrayType(type))
        return nilIsSized(nilGetArrayElementType(type));

    if (nilIsStructType(type))
    {
        NIL_STRUCT_TYPE *structType = (NIL_STRUCT_TYPE *)type;
        if (structType->isSized)
            return true;
        if (nilIsOpaque(type))
            return false;

        for (size_t i = 0; i < structType->elementCount; ++i)
        {
            NIL_TYPE *const t = structType->elements[i];
            if (!nilIsSized(t))
                return false;
        }

        // memorize that type is sized
        structType->isSized = true;
        return true;
    }

    assert(false);
    return false;
}

size_t nilGetPrimitiveSizeInBits(NIL_TYPE *type)
{
    switch (type->id)
    {
    case NIL_TYPE_FLOAT:
        return 32;
    case NIL_TYPE_DOUBLE:
        return 64;
    case NIL_TYPE_INTEGER:
        return nilGetIntegerBitWidth(type);
    default:
        return 0;
    }
}

NIL_INTEGER_BITS nilGetIntegerBitWidth(NIL_TYPE *type)
{
    assert(type->id == NIL_TYPE_INTEGER);
    if (type->id != NIL_TYPE_INTEGER)
        return 0;

    return ((NIL_INTEGER_TYPE *)type)->numBits;
}

size_t nilGetFunctionNumParams(NIL_TYPE *type)
{
    assert(type->id == NIL_TYPE_FUNCTION);
    if (type->id != NIL_TYPE_FUNCTION)
        return 0;

    return ((NIL_FUNCTION_TYPE *)type)->paramCount;
}

NIL_TYPE *nilGetFunctionParamType(NIL_TYPE *type, size_t i)
{
    assert(type->id == NIL_TYPE_FUNCTION);
    if (type->id != NIL_TYPE_FUNCTION)
        return NULL;

    return ((NIL_FUNCTION_TYPE *)type)->params[i];
}

bool nilIsFunctionVarArg(NIL_TYPE *type)
{
    assert(type->id == NIL_TYPE_FUNCTION);
    if (type->id != NIL_TYPE_FUNCTION)
        return false;

    return ((NIL_FUNCTION_TYPE *)type)->isVarArg;
}

// NT_STRING *nilGetStructName(NIL_TYPE *type)
// {
//     assert(type->id == NIL_TYPE_STRUCT);
//     if (type->id != NIL_TYPE_STRUCT)
//         return 0;

//     return ntRefString(((NIL_STRUCT_TYPE *)type)->name);
// }

size_t nilGetStructNumElements(NIL_TYPE *type)
{
    assert(type->id == NIL_TYPE_STRUCT);
    if (type->id != NIL_TYPE_STRUCT)
        return 0;

    return ((NIL_STRUCT_TYPE *)type)->elementCount;
}

NIL_TYPE *nilGetStructElementType(NIL_TYPE *type, size_t n)
{
    assert(type->id == NIL_TYPE_STRUCT);
    if (type->id != NIL_TYPE_STRUCT)
        return 0;

    assert(n >= 0 && n < nilGetStructNumElements(type));

    return ((NIL_STRUCT_TYPE *)type)->elements[n];
}

size_t nilGetArrayNumElements(NIL_TYPE *type)
{
    assert(type->id == NIL_TYPE_ARRAY);
    if (type->id != NIL_TYPE_ARRAY)
        return 0;

    return ((NIL_ARRAY_TYPE *)type)->numElements;
}

NIL_TYPE *nilGetArrayElementType(NIL_TYPE *type)
{
    assert(type->id == NIL_TYPE_ARRAY);
    if (type->id != NIL_TYPE_ARRAY)
        return 0;

    return ((NIL_ARRAY_TYPE *)type)->containedType;
}

NIL_TYPE *nilGetPointerTo(NIL_TYPE *pointeeType)
{
    NIL_CONTEXT *const c = pointeeType->context;
    assert(c);

    NIL_POINTER_TYPE *type;
    for (size_t i = 0; i < c->ptrTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->ptrTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIL_TYPE_POINTER);
        if (type->pointeeType == pointeeType)
            return (NIL_TYPE *)type;
    }

    type = (NIL_POINTER_TYPE *)ntMalloc(sizeof(NIL_POINTER_TYPE));
    type->type.id = NIL_TYPE_POINTER;
    type->type.context = c;
    type->pointeeType = pointeeType;
    ntArrayAdd(&c->ptrTypes, &type, sizeof(void *));

    return (NIL_TYPE *)type;
}

NIL_TYPE *nilGetPointeeType(NIL_TYPE *ptrType)
{
    assert(nilIsPointerType(ptrType));
    return ((NIL_POINTER_TYPE *)ptrType)->pointeeType;
}

bool nilIsValidElementType(NIL_TYPE *elementType)
{
    return !nilIsVoidType(elementType) && !nilIsLabelType(elementType) &&
           !nilIsFunctionType(elementType);
}

bool nilIsValidReturnType(NIL_TYPE *type)
{
    return !nilIsFunctionType(type) && !nilIsLabelType(type);
}

bool nilIsValidArgumentType(NIL_TYPE *type)
{
    return nilIsFirstClassType(type);
}

bool nilIsOpaque(NIL_TYPE *type)
{
    if (nilIsStructType(type))
        return !((NIL_STRUCT_TYPE *)type)->hasBody;
    if (nilIsPointerType(type))
        return !((NIL_POINTER_TYPE *)type)->pointeeType;
    assert(false);
    return false;
}

static bool serializeIntegerType(NIL_INTEGER_TYPE *type, NT_ARRAY *array)
{
    assert(type);
    assert(array);
    ntArrayAddVarint(array, type->numBits, NULL);
    return true;
}

static bool serializeFunctionType(NIL_FUNCTION_TYPE *type, NT_ARRAY *array)
{
    assert(type);
    assert(array);

    // return type
    assert(type->result);
    if (!nilSerializeType(type->result, array))
    {
        assert(false && "Fail to serialize return type!");
    }

    // params
    ntArrayAddVarint(array, type->paramCount, NULL);
    for (size_t i = 0; i < type->paramCount; ++i)
    {
        NIL_TYPE *paramType = type->params[i];
        assert(paramType);
        if (!nilSerializeType(paramType, array))
        {
            assert(false && "Fail to serialize param type!");
        }
    }

    // isVarArg
    ntArrayAddVarint(array, type->isVarArg, NULL);

    return true;
}

static bool serializePointerType(NIL_POINTER_TYPE *type, NT_ARRAY *array)
{
    assert(type);
    assert(array);

    return nilSerializeType(type->pointeeType, array);
}

static bool serializeStructType(NIL_STRUCT_TYPE *type, NT_ARRAY *array)
{
    assert(type);
    assert(array);

    // has body
    ntArrayAddVarint(array, type->hasBody, NULL);

    // has is sized
    ntArrayAddVarint(array, type->isSized, NULL);

    // element count
    ntArrayAddVarint(array, type->elementCount, NULL);

    // elements
    for (size_t i = 0; i < type->elementCount; ++i)
    {
        NIL_TYPE *elementType = type->elements[i];
        assert(elementType);
        nilSerializeType(elementType, array);
    }

    return true;
}

static bool serializeArrayType(NIL_ARRAY_TYPE *type, NT_ARRAY *array)
{
    assert(type);
    assert(array);

    // contained type
    assert(type->containedType);
    nilSerializeType(type->containedType, array);

    // elements count
    ntArrayAddVarint(array, type->numElements, NULL);

    return NULL;
}

bool nilSerializeType(NIL_TYPE *type, NT_ARRAY *array)
{
    assert(type);
    assert(array);

    ntArrayAddVarint(array, NIL_TYPE_BINARY, NULL);
    ntArrayAddVarint(array, type->id, NULL);

    switch (type->id)
    {
    case NIL_TYPE_FLOAT:
    case NIL_TYPE_DOUBLE:
    case NIL_TYPE_VOID:
    case NIL_TYPE_LABEL:
        break;
    case NIL_TYPE_INTEGER:
        return serializeIntegerType((NIL_INTEGER_TYPE *)type, array);
    case NIL_TYPE_FUNCTION:
        return serializeFunctionType((NIL_FUNCTION_TYPE *)type, array);
    case NIL_TYPE_POINTER:
        return serializePointerType((NIL_POINTER_TYPE *)type, array);
    case NIL_TYPE_STRUCT:
        return serializeStructType((NIL_STRUCT_TYPE *)type, array);
    case NIL_TYPE_ARRAY:
        return serializeArrayType((NIL_ARRAY_TYPE *)type, array);
    }

    return true;
}

void nilPrintType(NIL_TYPE *type)
{
    printf(GRN);
    switch (type->id)
    {
    case NIL_TYPE_VOID:
        printf("void");
        break;
    case NIL_ERROR_TYPE:
        printf("error");
        break;
    case NIL_TYPE_FLOAT:
        printf("float");
        break;
    case NIL_TYPE_DOUBLE:
        printf("double");
        break;
    case NIL_TYPE_LABEL:
        printf("label");
        break;

    case NIL_TYPE_INTEGER:
        printf("i%d", ((NIL_INTEGER_TYPE *)type)->numBits);
        break;
    case NIL_TYPE_FUNCTION: {
        NIL_FUNCTION_TYPE *f = (NIL_FUNCTION_TYPE *)type;
        nilPrintType(f->result);
        printf(" <- (");

        for (size_t i = 0; i < f->paramCount; ++i)
        {
            nilPrintType(f->params[i]);

            if (i + 1 < f->paramCount)
                printf(", ");
        }

        printf(")");
        break;
    }
    case NIL_TYPE_POINTER:
        nilPrintType(((NIL_POINTER_TYPE *)type)->pointeeType);
        printf("*");
        break;
    case NIL_TYPE_STRUCT: {
        NIL_STRUCT_TYPE *s = (NIL_STRUCT_TYPE *)type;
        printf("{");

        for (size_t i = 0; i < s->elementCount; ++i)
        {
            nilPrintType(s->elements[i]);

            if (i + 1 < s->elementCount)
                printf(", ");
        }

        printf("}");
        break;
    }
    case NIL_TYPE_ARRAY:
        nilPrintType(((NIL_ARRAY_TYPE *)type)->containedType);
        printf("[%zu]", ((NIL_ARRAY_TYPE *)type)->numElements);
        break;
    default:
        printf("INVALID_TYPE");
        break;
    }
    printf(reset);
}
