/*
MIT License

Copyright (c) 2022 Ezequias Silva <ezequiasmoises@gmail.com> and the Netuno
contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <assert.h>
#include <netuno/delegate.h>
#include <netuno/memory.h>
#include <netuno/module.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/type.h>

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    return obj1 == obj2;
}

static void freeDelegateType(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_TYPE_TYPE);

    NT_DELEGATE_TYPE *delegateType = (NT_DELEGATE_TYPE *)object;
    assert(IS_VALID_TYPE(delegateType));

    ntFreeObject((NT_OBJECT *)delegateType->type.typeName);
    ntFree(delegateType->params);
}

static const NT_STRING *typeToString(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_TYPE_TYPE);

    NT_TYPE *type = (NT_TYPE *)object;
    assert(IS_VALID_TYPE(type));

    ntRefObject((NT_OBJECT *)type->typeName);
    return type->typeName;
}

static NT_TYPE TYPE = {
    .object =
        (NT_OBJECT){
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_TYPE_TYPE,
    .typeName = NULL,
    .free = freeDelegateType,
    .string = typeToString,
    .equals = refEquals,
    .stackSize = sizeof(NT_REF),
    .instanceSize = sizeof(NT_DELEGATE_TYPE),
    .baseType = NULL,
};

const NT_TYPE *ntDelegateType(void)
{
    if (TYPE.object.type == NULL)
    {
        TYPE.object.type = ntType();
        TYPE.typeName = ntCopyString(U"DelegateType", 12);
        TYPE.baseType = ntObjectType();
        ntInitSymbolTable(&TYPE.fields, (NT_SYMBOL_TABLE *)&ntObjectType()->fields, STT_TYPE, 0);
    }
    return &TYPE;
}

static void freeDelegate(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_DELEGATE);
    NT_DELEGATE *delegate = (NT_DELEGATE *)object;
    ntFreeObject((NT_OBJECT *)delegate->name);
    delegate->name = NULL;
    delegate->native = false;
    delegate->func = NULL;
    delegate->addr = 0;
    delegate->sourceModule = NULL;
}

static const NT_STRING *delegateToString(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_DELEGATE);
    const NT_DELEGATE *delegate = (const NT_DELEGATE *)object;
    ntRefObject((NT_OBJECT *)delegate->name);
    return delegate->name;
}

const NT_DELEGATE_TYPE *ntCreateDelegateType(const NT_STRING *delegateTypeName,
                                             const NT_TYPE *returnType, size_t paramCount,
                                             const NT_PARAM *params)
{
    NT_DELEGATE_TYPE *delegateType = (NT_DELEGATE_TYPE *)ntCreateObject(ntDelegateType());

    delegateType->type.objectType = NT_OBJECT_DELEGATE;
    delegateType->type.typeName = delegateTypeName;
    delegateType->type.free = freeDelegate;
    delegateType->type.string = delegateToString;
    delegateType->type.equals = refEquals;
    delegateType->type.stackSize = sizeof(NT_REF);
    delegateType->type.instanceSize = sizeof(NT_DELEGATE);
    delegateType->type.baseType = ntObjectType();
    delegateType->paramCount = paramCount;
    delegateType->returnType = returnType;

    delegateType->params = ntMalloc(sizeof(NT_PARAM) * paramCount);
    ntMemcpy(delegateType->params, params, sizeof(NT_PARAM) * paramCount);
    return delegateType;
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

const NT_DELEGATE *ntDelegate(const NT_DELEGATE_TYPE *delegateType, const NT_MODULE *module,
                              size_t addr, const NT_STRING *name)
{
    assert(delegateType);
    assert(IS_VALID_OBJECT(delegateType));
    assert(IS_VALID_TYPE(delegateType));
    assert(delegateType->type.object.type->objectType == NT_OBJECT_TYPE_TYPE);
    assert(delegateType->type.objectType == NT_OBJECT_DELEGATE);

    assert(module);
    assert(IS_VALID_OBJECT(module));
    assert(module->type.object.type->objectType == NT_OBJECT_TYPE_TYPE);
    assert(module->type.objectType == NT_OBJECT_MODULE);

    NT_DELEGATE *delegate = (NT_DELEGATE *)ntCreateObject((NT_TYPE *)delegateType);
    delegate->native = false;
    delegate->addr = addr;
    delegate->sourceModule = module;
    delegate->name = name;

    return delegate;
}

const NT_DELEGATE *ntNativeDelegate(const NT_DELEGATE_TYPE *delegateType, nativeFun func,
                                    const NT_STRING *name)
{
    assert(delegateType);
    assert(IS_VALID_OBJECT(delegateType));
    assert(IS_VALID_TYPE(delegateType));
    assert(delegateType->type.object.type->objectType == NT_OBJECT_TYPE_TYPE);
    assert(delegateType->type.objectType == NT_OBJECT_DELEGATE);

    assert(func);

    NT_DELEGATE *delegate = (NT_DELEGATE *)ntCreateObject((NT_TYPE *)delegateType);
    delegate->native = true;
    delegate->func = func;
    delegate->name = name;

    return delegate;
}
