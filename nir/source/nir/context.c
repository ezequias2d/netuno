#include "netuno/nir/context.h"
#include "netuno/nir/type.h"
#include "nir/plist.h"
#include "pcontext.h"
#include "ptype.h"
#include <assert.h>
#include <netuno/array.h>
#include <netuno/memory.h>
#include <netuno/string.h>
#include <netuno/table.h>

static NIR_TYPE *createPrimitiveType(NIR_CONTEXT *context, NIR_TYPE_ID id)
{
    NIR_TYPE *type = (NIR_TYPE *)ntMalloc(sizeof(NIR_TYPE));
    type->id = id;
    type->context = context;
    return type;
}

static NIR_POINTER_TYPE *createOpaquePtrType(NIR_CONTEXT *context)
{
    NIR_POINTER_TYPE *type =
        (NIR_POINTER_TYPE *)ntMalloc(sizeof(NIR_POINTER_TYPE));
    type->type.id = NIR_TYPE_POINTER;
    type->type.context = context;
    type->pointeeType = NULL;
    ntArrayAdd(&context->ptrTypes, type, sizeof(void *));
    return type;
}

NIR_CONTEXT *nirCreateContext(void)
{
    NIR_CONTEXT *c = ntMalloc(sizeof(NIR_CONTEXT));

    ntInitArray(&c->integerTypes);
    ntInitArray(&c->functionTypes);
    ntInitArray(&c->arrayTypes);
    ntInitArray(&c->ptrTypes);
    ntInitArray(&c->structTypes);

    *c = (NIR_CONTEXT){
        .errorType = createPrimitiveType(c, NIR_ERROR_TYPE),
        .voidType = createPrimitiveType(c, NIR_TYPE_VOID),
        .labelType = createPrimitiveType(c, NIR_TYPE_LABEL),
        .floatType = createPrimitiveType(c, NIR_TYPE_FLOAT),
        .doubleType = createPrimitiveType(c, NIR_TYPE_DOUBLE),
        .opaquePtrType = createOpaquePtrType(c),
        .prefixes = ntCreateTable(),
    };

    return c;
}

static void freeType(NIR_TYPE *type)
{
    assert(type->id >= NIR_TYPE_FLOAT && type->id <= NIR_TYPE_ARRAY);
    ntFree(type);
}

static void freeInteger(NIR_TYPE *type)
{
    assert(type);
    assert(type->id == NIR_TYPE_INTEGER);
    freeType(type);
}

static void freeFunction(NIR_TYPE *type)
{
    assert(type);
    assert(type->id == NIR_TYPE_FUNCTION);

    NIR_FUNCTION_TYPE *function = (NIR_FUNCTION_TYPE *)type;
    ntFree(function->params);

    freeType(type);
}

static void freeArray(NIR_TYPE *type)
{
    assert(type);
    assert(type->id == NIR_TYPE_ARRAY);

    freeType(type);
}

static void freePtr(NIR_TYPE *type)
{
    assert(type);
    assert(type->id == NIR_TYPE_POINTER);

    freeType(type);
}

static void freeStruct(NIR_TYPE *type)
{
    assert(type);
    assert(type->id == NIR_TYPE_STRUCT);

    NIR_STRUCT_TYPE *structType = (NIR_STRUCT_TYPE *)type;

    for (size_t i = 0; i < structType->elementCount; ++i)
    {
        freeType(structType->elements[i]);
    }
    ntFree(structType->elements);

    // ntUnrefString(structType->name);
    freeType(type);
}

static void freePrefixValue(NT_STRING *key, void *value, void *userdata)
{
    assert(key);
    assert(userdata == NULL);
    ntFree(value);
}

void nirDestroyContext(NIR_CONTEXT *c)
{
    ntFree(c->errorType);
    ntFree(c->voidType);
    ntFree(c->labelType);
    ntFree(c->floatType);
    ntFree(c->doubleType);

    for (size_t i = 0; i < c->integerTypes.count; i += sizeof(void *))
    {
        NIR_TYPE *type;
        const bool result = ntArrayGet(&c->integerTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeInteger(type);
    }

    for (size_t i = 0; i < c->functionTypes.count; i += sizeof(void *))
    {
        NIR_TYPE *type;
        const bool result = ntArrayGet(&c->functionTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeFunction(type);
    }

    for (size_t i = 0; i < c->arrayTypes.count; i += sizeof(void *))
    {
        NIR_TYPE *type;
        const bool result = ntArrayGet(&c->arrayTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeArray(type);
    }

    for (size_t i = 0; i < c->ptrTypes.count; i += sizeof(void *))
    {
        NIR_TYPE *type;
        const bool result = ntArrayGet(&c->ptrTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freePtr(type);
    }

    for (size_t i = 0; i < c->structTypes.count; i += sizeof(void *))
    {
        NIR_TYPE *type;
        const bool result = ntArrayGet(&c->structTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeStruct(type);
    }

    ntDeinitArray(&c->integerTypes);
    ntDeinitArray(&c->functionTypes);
    ntDeinitArray(&c->arrayTypes);
    ntDeinitArray(&c->ptrTypes);
    ntDeinitArray(&c->structTypes);

    ntTableForAll(c->prefixes, freePrefixValue, NULL);
    ntFreeTable(c->prefixes);

    ntFree(c);
}

NT_STRING *nirGetPrefixedId(NIR_CONTEXT *context, const char_t *prefix)
{
    uint64_t *pN;

    NT_STRING *sprefix = ntCopyCString(prefix);

    if (!ntTableGet(context->prefixes, sprefix, (void **)&pN))
    {
        pN = ntMalloc(sizeof(uint64_t));
        const bool result =
            ntTableSet(context->prefixes, ntRefString(sprefix), pN);
        assert(result);

        *pN = 0;
    }

    NT_STRING *result = ntConcat(sprefix, ntU64ToString((*pN)++));
    ntUnrefString(sprefix);
    return result;
}
