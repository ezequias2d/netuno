#include "netuno/nir/type.h"
#include "colors.h"
#include "netuno/array.h"
#include "netuno/memory.h"
#include "netuno/nir/context.h"
#include "netuno/string.h"
#include "pcontext.h"
#include "ptype.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

NIR_TYPE *nirGetErrorType(NIR_CONTEXT *c)
{
    return c->errorType;
}

NIR_TYPE *nirGetVoidType(NIR_CONTEXT *c)
{
    return c->voidType;
}

NIR_TYPE *nirGetLabelType(NIR_CONTEXT *c)
{
    return c->labelType;
}

NIR_TYPE *nirGetFloatType(NIR_CONTEXT *c)
{
    return c->floatType;
}

NIR_TYPE *nirGetDoubleType(NIR_CONTEXT *c)
{
    return c->doubleType;
}

NIR_TYPE *nirGetOpaquePointerType(NIR_CONTEXT *c)
{
    assert(c);
    return (NIR_TYPE *)c->opaquePtrType;
}

NIR_TYPE *nirGetIntegerType(NIR_CONTEXT *c, NIR_INTEGER_BITS numBits)
{
    NIR_INTEGER_TYPE *type = NULL;
    for (size_t i = 0; i < c->integerTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->integerTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIR_TYPE_INTEGER);
        if (type->numBits == numBits)
            return (NIR_TYPE *)type;
    }

    type = (NIR_INTEGER_TYPE *)ntMalloc(sizeof(NIR_INTEGER_TYPE));
    type->type.id = NIR_TYPE_INTEGER;
    type->type.context = c;
    type->numBits = numBits;
    ntArrayAdd(&c->integerTypes, &type, sizeof(void *));

    return (NIR_TYPE *)type;
}

NIR_TYPE *nirGetInt1Type(NIR_CONTEXT *c)
{
    return nirGetIntegerType(c, 1);
}

NIR_TYPE *nirGetInt8Type(NIR_CONTEXT *c)
{
    return nirGetIntegerType(c, 8);
}

NIR_TYPE *nirGetInt16Type(NIR_CONTEXT *c)
{
    return nirGetIntegerType(c, 16);
}

NIR_TYPE *nirGetInt32Type(NIR_CONTEXT *c)
{
    return nirGetIntegerType(c, 32);
}

NIR_TYPE *nirGetInt64Type(NIR_CONTEXT *c)
{
    return nirGetIntegerType(c, 64);
}

NIR_TYPE *nirGetFunctionType(NIR_CONTEXT *c, NIR_TYPE *result,
                             size_t paramCount, NIR_TYPE *const *params,
                             bool isVarArg)
{
    NIR_FUNCTION_TYPE *type;
    for (size_t i = 0; i < c->functionTypes.count; i += sizeof(void *))
    {
        const bool getResult = ntArrayGet(&c->functionTypes, i, &type,
                                          sizeof(void *)) == sizeof(void *);
        assert(getResult);
        assert(type->type.id == NIR_TYPE_FUNCTION);
        if (type->result == result && type->isVarArg == isVarArg &&
            type->paramCount == paramCount &&
            memcmp(type->params, params, sizeof(void *) * paramCount) == 0)
            return (NIR_TYPE *)type;
    }

    type = (NIR_FUNCTION_TYPE *)ntMalloc(sizeof(NIR_FUNCTION_TYPE));
    type->type.id = NIR_TYPE_FUNCTION;
    type->type.context = c;
    type->result = result;
    type->paramCount = paramCount;
    type->params = (NIR_TYPE **)ntMalloc(sizeof(void *) * paramCount);
    ntMemcpy(type->params, params, sizeof(void *) * paramCount);
    ntArrayAdd(&c->functionTypes, &type, sizeof(void *));

    return (NIR_TYPE *)type;
}

NIR_TYPE *nirGetArrayType(NIR_CONTEXT *c, NIR_TYPE *elementType,
                          uint64_t numElements)
{
    NIR_ARRAY_TYPE *type;
    for (size_t i = 0; i < c->arrayTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->arrayTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIR_TYPE_ARRAY);
        if (type->numElements == numElements &&
            type->containedType == elementType)
            return (NIR_TYPE *)type;
    }

    type = (NIR_ARRAY_TYPE *)ntMalloc(sizeof(NIR_ARRAY_TYPE));
    type->type.id = NIR_TYPE_ARRAY;
    type->type.context = c;
    type->containedType = elementType;
    type->numElements = numElements;
    ntArrayAdd(&c->arrayTypes, &type, sizeof(void *));

    return (NIR_TYPE *)type;
}

NIR_TYPE *nirGetFloatPtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(c->floatType);
}

NIR_TYPE *nirGetDoublePtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(c->doubleType);
}

NIR_TYPE *nirGetIntegerPtrType(NIR_CONTEXT *c, NIR_INTEGER_BITS n)
{
    return nirGetPointerTo(nirGetIntegerType(c, n));
}

NIR_TYPE *nirGetInt1PtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(nirGetInt1Type(c));
}

NIR_TYPE *nirGetInt8PtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(nirGetInt8Type(c));
}

NIR_TYPE *nirGetInt16PtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(nirGetInt16Type(c));
}

NIR_TYPE *nirGetInt32PtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(nirGetInt32Type(c));
}

NIR_TYPE *nirGetInt64PtrType(NIR_CONTEXT *c)
{
    return nirGetPointerTo(nirGetInt64Type(c));
}

NIR_TYPE *nirGetStructType(NIR_CONTEXT *c, size_t elementCount,
                           NIR_TYPE *const *elementTypes)
{
    NIR_STRUCT_TYPE *type;
    for (size_t i = 0; i < c->arrayTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->structTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIR_TYPE_STRUCT);
        if (type->elementCount == elementCount &&
            memcmp(type->elements, elementTypes,
                   sizeof(NIR_TYPE *) * elementCount) == 0)
            return (NIR_TYPE *)type;
    }

    type = (NIR_STRUCT_TYPE *)ntMalloc(sizeof(NIR_STRUCT_TYPE));
    type->type.id = NIR_TYPE_STRUCT;
    type->type.context = c;
    type->hasBody = true;
    type->isSized = true;
    type->elementCount = elementCount;
    type->elements = (NIR_TYPE **)ntMalloc(sizeof(NIR_TYPE *) * elementCount);
    ntMemcpy(type->elements, elementTypes, sizeof(NIR_TYPE *) * elementCount);
    ntArrayAdd(&c->structTypes, &type, sizeof(void *));

    return (NIR_TYPE *)type;
}

NIR_CONTEXT *nirGetTypeContext(NIR_TYPE *type)
{
    return type->context;
}

NIR_TYPE_ID nirGetTypeID(NIR_TYPE *type)
{
    return type->id;
}

bool nirIsVoidType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_VOID;
}

bool nirIsLabelType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_LABEL;
}

bool nirIsFloatType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_FLOAT;
}

bool nirIsDoubleType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_DOUBLE;
}

bool nirIsIntegerType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_INTEGER;
}

bool nirIsIntegerNType(NIR_TYPE *type, enum_t n)
{
    return type->id == NIR_TYPE_INTEGER &&
           ((NIR_INTEGER_TYPE *)type)->numBits == n;
}

bool nirIsFunctionType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_FUNCTION;
}

bool nirIsStructType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_STRUCT;
}

bool nirIsArrayType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_ARRAY;
}

bool nirIsPointerType(NIR_TYPE *type)
{
    return type->id == NIR_TYPE_POINTER;
}

bool nirIsFirstClassType(NIR_TYPE *type)
{
    return type->id != NIR_TYPE_VOID && type->id != NIR_TYPE_FUNCTION;
}

bool nirIsSingleValueType(NIR_TYPE *type)
{
    return nirIsFloatType(type) || nirIsIntegerType(type) ||
           nirIsDoubleType(type) || nirIsPointerType(type);
}

bool nirIsAggregateType(NIR_TYPE *type)
{
    return nirIsStructType(type) || nirIsArrayType(type);
}

bool nirIsSized(NIR_TYPE *type)
{
    if (nirIsIntegerType(type) || nirIsFloatType(type) ||
        nirIsDoubleType(type) || nirIsPointerType(type))
        return true;

    if (!nirIsStructType(type) && !nirIsArrayType(type))
        return false;

    if (nirIsArrayType(type))
        return nirIsSized(nirGetArrayElementType(type));

    if (nirIsStructType(type))
    {
        NIR_STRUCT_TYPE *structType = (NIR_STRUCT_TYPE *)type;
        if (structType->isSized)
            return true;
        if (nirIsOpaque(type))
            return false;

        for (size_t i = 0; i < structType->elementCount; ++i)
        {
            NIR_TYPE *const t = structType->elements[i];
            if (!nirIsSized(t))
                return false;
        }

        // memorize that type is sized
        structType->isSized = true;
        return true;
    }

    assert(false);
    return false;
}

size_t nirGetPrimitiveSizeInBits(NIR_TYPE *type)
{
    switch (type->id)
    {
    case NIR_TYPE_FLOAT:
        return 32;
    case NIR_TYPE_DOUBLE:
        return 64;
    case NIR_TYPE_INTEGER:
        return nirGetIntegerBitWidth(type);
    default:
        return 0;
    }
}

NIR_INTEGER_BITS nirGetIntegerBitWidth(NIR_TYPE *type)
{
    assert(type->id == NIR_TYPE_INTEGER);
    if (type->id != NIR_TYPE_INTEGER)
        return 0;

    return ((NIR_INTEGER_TYPE *)type)->numBits;
}

size_t nirGetFunctionNumParams(NIR_TYPE *type)
{
    assert(type->id == NIR_TYPE_FUNCTION);
    if (type->id != NIR_TYPE_FUNCTION)
        return 0;

    return ((NIR_FUNCTION_TYPE *)type)->paramCount;
}

NIR_TYPE *nirGetFunctionParamType(NIR_TYPE *type, size_t i)
{
    assert(type->id == NIR_TYPE_FUNCTION);
    if (type->id != NIR_TYPE_FUNCTION)
        return NULL;

    return ((NIR_FUNCTION_TYPE *)type)->params[i];
}

bool nirIsFunctionVarArg(NIR_TYPE *type)
{
    assert(type->id == NIR_TYPE_FUNCTION);
    if (type->id != NIR_TYPE_FUNCTION)
        return false;

    return ((NIR_FUNCTION_TYPE *)type)->isVarArg;
}

// NT_STRING *nirGetStructName(NIR_TYPE *type)
// {
//     assert(type->id == NIR_TYPE_STRUCT);
//     if (type->id != NIR_TYPE_STRUCT)
//         return 0;

//     return ntRefString(((NIR_STRUCT_TYPE *)type)->name);
// }

size_t nirGetStructNumElements(NIR_TYPE *type)
{
    assert(type->id == NIR_TYPE_STRUCT);
    if (type->id != NIR_TYPE_STRUCT)
        return 0;

    return ((NIR_STRUCT_TYPE *)type)->elementCount;
}

NIR_TYPE *nirGetStructElementType(NIR_TYPE *type, size_t n)
{
    assert(type->id == NIR_TYPE_STRUCT);
    if (type->id != NIR_TYPE_STRUCT)
        return 0;

    assert(n >= 0 && n < nirGetStructNumElements(type));

    return ((NIR_STRUCT_TYPE *)type)->elements[n];
}

size_t nirGetArrayNumElements(NIR_TYPE *type)
{
    assert(type->id == NIR_TYPE_ARRAY);
    if (type->id != NIR_TYPE_ARRAY)
        return 0;

    return ((NIR_ARRAY_TYPE *)type)->numElements;
}

NIR_TYPE *nirGetArrayElementType(NIR_TYPE *type)
{
    assert(type->id == NIR_TYPE_ARRAY);
    if (type->id != NIR_TYPE_ARRAY)
        return 0;

    return ((NIR_ARRAY_TYPE *)type)->containedType;
}

NIR_TYPE *nirGetPointerTo(NIR_TYPE *pointeeType)
{
    NIR_CONTEXT *const c = pointeeType->context;
    assert(c);

    NIR_POINTER_TYPE *type;
    for (size_t i = 0; i < c->ptrTypes.count; i += sizeof(void *))
    {
        const bool result = ntArrayGet(&c->ptrTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        assert(type->type.id == NIR_TYPE_POINTER);
        if (type->pointeeType == pointeeType)
            return (NIR_TYPE *)type;
    }

    type = (NIR_POINTER_TYPE *)ntMalloc(sizeof(NIR_POINTER_TYPE));
    type->type.id = NIR_TYPE_POINTER;
    type->type.context = c;
    type->pointeeType = pointeeType;
    ntArrayAdd(&c->ptrTypes, &type, sizeof(void *));

    return (NIR_TYPE *)type;
}

NIR_TYPE *nirGetPointeeType(NIR_TYPE *ptrType)
{
    assert(nirIsPointerType(ptrType));
    return ((NIR_POINTER_TYPE *)ptrType)->pointeeType;
}

bool nirIsValidElementType(NIR_TYPE *elementType)
{
    return !nirIsVoidType(elementType) && !nirIsLabelType(elementType) &&
           !nirIsFunctionType(elementType);
}

bool nirIsValidReturnType(NIR_TYPE *type)
{
    return !nirIsFunctionType(type) && !nirIsLabelType(type);
}

bool nirIsValidArgumentType(NIR_TYPE *type)
{
    return nirIsFirstClassType(type);
}

bool nirIsOpaque(NIR_TYPE *type)
{
    if (nirIsStructType(type))
        return !((NIR_STRUCT_TYPE *)type)->hasBody;
    if (nirIsPointerType(type))
        return !((NIR_POINTER_TYPE *)type)->pointeeType;
    assert(false);
    return false;
}

void nirPrintType(NIR_TYPE *type)
{
    printf(GRN);
    switch (type->id)
    {
    case NIR_TYPE_VOID:
        printf("void");
        break;
    case NIR_ERROR_TYPE:
        printf("error");
        break;
    case NIR_TYPE_FLOAT:
        printf("float");
        break;
    case NIR_TYPE_DOUBLE:
        printf("double");
        break;
    case NIR_TYPE_LABEL:
        printf("label");
        break;

    case NIR_TYPE_INTEGER:
        printf("i%d", ((NIR_INTEGER_TYPE *)type)->numBits);
        break;
    case NIR_TYPE_FUNCTION: {
        NIR_FUNCTION_TYPE *f = (NIR_FUNCTION_TYPE *)type;
        nirPrintType(f->result);
        printf(" <- (");

        for (size_t i = 0; i < f->paramCount; ++i)
        {
            nirPrintType(f->params[i]);

            if (i + 1 < f->paramCount)
                printf(", ");
        }

        printf(")");
        break;
    }
    case NIR_TYPE_POINTER:
        nirPrintType(((NIR_POINTER_TYPE *)type)->pointeeType);
        printf("*");
        break;
    case NIR_TYPE_STRUCT: {
        NIR_STRUCT_TYPE *s = (NIR_STRUCT_TYPE *)type;
        printf("{");

        for (size_t i = 0; i < s->elementCount; ++i)
        {
            nirPrintType(s->elements[i]);

            if (i + 1 < s->elementCount)
                printf(", ");
        }

        printf("}");
        break;
    }
    case NIR_TYPE_ARRAY:
        nirPrintType(((NIR_ARRAY_TYPE *)type)->containedType);
        printf("[%zu]", ((NIR_ARRAY_TYPE *)type)->numElements);
        break;
    default:
        printf("INVALID_TYPE");
        break;
    }
    printf(reset);
}
