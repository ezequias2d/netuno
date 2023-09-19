#include "netuno/nil/context.h"
#include "netuno/nil/type.h"
#include "pcontext.h"
#include "plist.h"
#include "ptype.h"
#include <assert.h>
#include <netuno/array.h>
#include <netuno/memory.h>
#include <netuno/string.h>
#include <netuno/table.h>

static NIL_TYPE *createPrimitiveType(NIL_CONTEXT *context, NIL_TYPE_ID id)
{
    NIL_TYPE *type = (NIL_TYPE *)ntMalloc(sizeof(NIL_TYPE));
    type->id = id;
    type->context = context;
    return type;
}

static NIL_POINTER_TYPE *createOpaquePtrType(NIL_CONTEXT *context)
{
    NIL_POINTER_TYPE *type =
        (NIL_POINTER_TYPE *)ntMalloc(sizeof(NIL_POINTER_TYPE));
    type->type.id = NIL_TYPE_POINTER;
    type->type.context = context;
    type->pointeeType = NULL;
    ntArrayAdd(&context->ptrTypes, type, sizeof(void *));
    return type;
}

NIL_CONTEXT *nilCreateContext(void)
{
    NIL_CONTEXT *c = ntMalloc(sizeof(NIL_CONTEXT));

    ntInitArray(&c->integerTypes);
    ntInitArray(&c->functionTypes);
    ntInitArray(&c->arrayTypes);
    ntInitArray(&c->ptrTypes);
    ntInitArray(&c->structTypes);

    *c = (NIL_CONTEXT){
        .errorType = createPrimitiveType(c, NIL_ERROR_TYPE),
        .voidType = createPrimitiveType(c, NIL_TYPE_VOID),
        .labelType = createPrimitiveType(c, NIL_TYPE_LABEL),
        .floatType = createPrimitiveType(c, NIL_TYPE_FLOAT),
        .doubleType = createPrimitiveType(c, NIL_TYPE_DOUBLE),
        .opaquePtrType = createOpaquePtrType(c),
        .prefixes = ntCreateTable(),
    };

    return c;
}

static void freeType(NIL_TYPE *type)
{
    assert(type->id >= NIL_TYPE_FLOAT && type->id <= NIL_TYPE_ARRAY);
    ntFree(type);
}

static void freeInteger(NIL_TYPE *type)
{
    assert(type);
    assert(type->id == NIL_TYPE_INTEGER);
    freeType(type);
}

static void freeFunction(NIL_TYPE *type)
{
    assert(type);
    assert(type->id == NIL_TYPE_FUNCTION);

    NIL_FUNCTION_TYPE *function = (NIL_FUNCTION_TYPE *)type;
    ntFree(function->params);

    freeType(type);
}

static void freeArray(NIL_TYPE *type)
{
    assert(type);
    assert(type->id == NIL_TYPE_ARRAY);

    freeType(type);
}

static void freePtr(NIL_TYPE *type)
{
    assert(type);
    assert(type->id == NIL_TYPE_POINTER);

    freeType(type);
}

static void freeStruct(NIL_TYPE *type)
{
    assert(type);
    assert(type->id == NIL_TYPE_STRUCT);

    NIL_STRUCT_TYPE *structType = (NIL_STRUCT_TYPE *)type;

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

void nilDestroyContext(NIL_CONTEXT *c)
{
    ntFree(c->errorType);
    ntFree(c->voidType);
    ntFree(c->labelType);
    ntFree(c->floatType);
    ntFree(c->doubleType);

    for (size_t i = 0; i < c->integerTypes.count; i += sizeof(void *))
    {
        NIL_TYPE *type;
        const bool result = ntArrayGet(&c->integerTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeInteger(type);
    }

    for (size_t i = 0; i < c->functionTypes.count; i += sizeof(void *))
    {
        NIL_TYPE *type;
        const bool result = ntArrayGet(&c->functionTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeFunction(type);
    }

    for (size_t i = 0; i < c->arrayTypes.count; i += sizeof(void *))
    {
        NIL_TYPE *type;
        const bool result = ntArrayGet(&c->arrayTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freeArray(type);
    }

    for (size_t i = 0; i < c->ptrTypes.count; i += sizeof(void *))
    {
        NIL_TYPE *type;
        const bool result = ntArrayGet(&c->ptrTypes, i, &type,
                                       sizeof(void *)) == sizeof(void *);
        assert(result);
        freePtr(type);
    }

    for (size_t i = 0; i < c->structTypes.count; i += sizeof(void *))
    {
        NIL_TYPE *type;
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

NT_STRING *nilGetPrefixedId(NIL_CONTEXT *context, const char_t *prefix)
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
