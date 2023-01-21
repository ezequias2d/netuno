#include "netuno/gc.h"
#include <assert.h>
#include <netuno/delegate.h>
#include <netuno/memory.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/type.h>

static void freeDelegate(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_DELEGATE);
    NT_DELEGATE *delegate = (NT_DELEGATE *)object;
    ntFreeObject((NT_OBJECT *)delegate->name);
    delegate->name = NULL;
    delegate->native = false;
    delegate->func = NULL;
    delegate->addr = 0;
    delegate->sourceChunk = NULL;
}

static const NT_STRING *delegateToString(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_DELEGATE);
    const NT_DELEGATE *delegate = (const NT_DELEGATE *)object;
    return delegate->name;
}

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    return obj1 == obj2;
}

const NT_DELEGATE_TYPE *ntCreateDelegateType(const NT_STRING *delegateTypeName,
                                             const NT_TYPE *returnType, size_t paramCount,
                                             const NT_PARAM *params)
{
    NT_DELEGATE_TYPE *type =
        (NT_DELEGATE_TYPE *)ntMalloc(sizeof(NT_DELEGATE_TYPE) + sizeof(NT_PARAM) * paramCount);

    *type = (NT_DELEGATE_TYPE){
        .type =
            (NT_TYPE){
                .objectType = NT_OBJECT_DELEGATE,
                .typeName = delegateTypeName,
                .free = freeDelegate,
                .string = delegateToString,
                .equals = refEquals,
                .stackSize = sizeof(uint32_t),
                .instanceSize = sizeof(NT_DELEGATE),
            },
        .paramCount = paramCount,
        .returnType = returnType,
    };
    ntMemcpy((void *)type->params, params, sizeof(NT_PARAM) * paramCount);
    return type;
}

char_t *ntDelegateTypeName(const NT_TYPE *returnType, size_t paramCount, const NT_PARAM *params)
{
    NT_ARRAY array;
    ntInitArray(&array);

    const char_t *const delegate = U"delegate(";
    ntArrayAdd(&array, delegate, ntStrLen(delegate));
    for (size_t i_param = 0; i_param < paramCount; ++i_param)
    {
        const NT_PARAM *param = &params[i_param];
        assert(param->type);
        const NT_STRING *typeName = param->type->typeName;
        ntArrayAdd(&array, typeName->chars, typeName->length * sizeof(char_t));
    }
    ntArrayAdd(&array, L")", sizeof(char_t));

    if (returnType)
    {
        assert(returnType->typeName);
        ntArrayAdd(&array, L":", sizeof(char_t));
        ntArrayAdd(&array, returnType->typeName->chars, returnType->typeName->length);
    }

    const char_t term = '\0';
    ntArrayAdd(&array, &term, sizeof(char_t));

    return ntRealloc(array.data, array.count);
}

const NT_DELEGATE *ntDelegate(const NT_DELEGATE_TYPE *delegateType, const NT_CHUNK *chunk,
                              size_t addr, const NT_STRING *name)
{
    NT_DELEGATE *delegate = (NT_DELEGATE *)ntCreateObject((NT_TYPE *)delegateType);
    delegate->native = false;
    delegate->addr = addr;
    delegate->sourceChunk = chunk;
    delegate->name = name;

    return delegate;
}
