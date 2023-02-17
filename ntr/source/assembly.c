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
#include <netuno/assembly.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    assert(obj1);
    assert(IS_VALID_OBJECT(obj1));
    assert(obj1->type->objectType == NT_OBJECT_MODULE);

    assert(obj2);
    assert(IS_VALID_OBJECT(obj2));
    assert(obj2->type->objectType == NT_OBJECT_MODULE);

    return obj1 == obj2;
}

static void freeAssembly(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_ASSEMBLY);

    NT_ASSEMBLY *assembly = (NT_ASSEMBLY *)object;

    for (size_t i = 0; i < assembly->objects->count / sizeof(NT_REF); ++i)
    {
        NT_OBJECT *object;
        ntArrayGet(assembly->objects, i * sizeof(NT_REF), &object, sizeof(NT_REF));
        ntForceFreeObject(object);
    }

    ntFreeArray(assembly->objects);
}

static const NT_STRING *assemblyToString(NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));
    assert(object->type->objectType == NT_OBJECT_ASSEMBLY);

    return object->type->typeName;
}

static NT_TYPE ASSEMBLY_TYPE = {
    .object =
        {
            .type = NULL,
            .refCount = 0,
        },
    .objectType = NT_OBJECT_ASSEMBLY,
    .typeName = NULL,
    .free = freeAssembly,
    .string = assemblyToString,
    .equals = refEquals,
    .stackSize = sizeof(NT_REF),
    .instanceSize = sizeof(NT_ASSEMBLY),
    .baseType = NULL,
};

const NT_TYPE *ntAssemblyType(void)
{
    if (ASSEMBLY_TYPE.object.type == NULL)
    {
        ASSEMBLY_TYPE.object.type = ntType();
        ASSEMBLY_TYPE.typeName = ntCopyString(U"Assembly", 3);
        ASSEMBLY_TYPE.baseType = ntObjectType();
        ntInitSymbolTable(&ASSEMBLY_TYPE.fields, (NT_SYMBOL_TABLE *)&ntType()->fields, STT_TYPE, 0);
    }
    return &ASSEMBLY_TYPE;
}

NT_ASSEMBLY *ntCreateAssembly(void)
{
    NT_ASSEMBLY *assembly = (NT_ASSEMBLY *)ntCreateObject(ntAssemblyType());
    assembly->objects = ntCreateArray();
    return assembly;
}

static const NT_DELEGATE_TYPE *findDelegateType(NT_ASSEMBLY *assembly,
                                                const NT_STRING *delegateName)
{
    for (size_t i = 0; i < assembly->objects->count / sizeof(NT_REF); ++i)
    {
        NT_OBJECT *object;
        ntArrayGet(assembly->objects, i * sizeof(NT_REF), &object, sizeof(NT_REF));

        assert(object);
        assert(IS_VALID_OBJECT(object));

        if (object->type != ntDelegateType())
            continue;

        const NT_DELEGATE_TYPE *const delegateType = (NT_DELEGATE_TYPE *)object;
        if (ntEquals((NT_OBJECT *)delegateType->type.typeName, (NT_OBJECT *)delegateName))
            return delegateType;
    }
    return NULL;
}

const NT_DELEGATE_TYPE *ntTakeDelegateType(NT_ASSEMBLY *assembly, const NT_TYPE *returnType,
                                           size_t paramCount, const NT_PARAM *params)
{
    assert(assembly);

    const NT_STRING *delegateName;
    {
        char_t *name = ntDelegateTypeName(returnType, paramCount, params);
        const size_t len = ntStrLen(name);
        delegateName = ntTakeString(name, len);
    }

    const NT_DELEGATE_TYPE *delegateType = findDelegateType(assembly, delegateName);
    if (delegateType != NULL)
        return delegateType;

    delegateType = ntCreateDelegateType(delegateName, returnType, paramCount, params);
    ntAddConstantObject(assembly, (NT_OBJECT *)delegateType);
    return delegateType;
}

uint64_t ntAddConstantObject(NT_ASSEMBLY *assembly, NT_OBJECT *object)
{
    assert(object);
    assert(IS_VALID_OBJECT(object));

    size_t offset;
    uint64_t result;

    if (!ntArrayFind(assembly->objects, &object, sizeof(NT_OBJECT *), &offset))
    {
        ntMakeConstant(object);
        ntArrayAdd(assembly->objects, &object, sizeof(NT_OBJECT *));
        offset = assembly->objects->count - sizeof(NT_OBJECT *);
    }

    result = offset / sizeof(NT_OBJECT *);
    assert(result * sizeof(NT_OBJECT *) == offset);

    return result;
}

NT_OBJECT *ntGetConstantObject(const NT_ASSEMBLY *assembly, uint64_t constant)
{
    NT_OBJECT *object = NULL;
    const bool result = ntArrayGet(assembly->objects, constant * sizeof(NT_OBJECT *), &object,
                                   sizeof(NT_OBJECT *)) == sizeof(NT_DELEGATE *);
    assert(result);
    return object;
}
