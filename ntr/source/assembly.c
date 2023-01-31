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
        (NT_OBJECT){
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
};

const NT_TYPE *ntAssemblyType(void)
{
    if (ASSEMBLY_TYPE.object.type == NULL)
        ASSEMBLY_TYPE.object.type = ntType();
    if (ASSEMBLY_TYPE.typeName == NULL)
        ASSEMBLY_TYPE.typeName = ntCopyString(U"Assembly", 3);
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

    uint64_t offset;

    if (ntArrayFind(assembly->objects, &object, sizeof(NT_OBJECT *), &offset))
        return offset;

    ntMakeConstant(object);
    ntArrayAdd(assembly->objects, &object, sizeof(NT_OBJECT *));
    return assembly->objects->count - sizeof(NT_TYPE *);
}

NT_OBJECT *ntGetConstantObject(const NT_ASSEMBLY *assembly, uint64_t constant)
{
    NT_OBJECT *object = NULL;
    const bool result = ntArrayGet(assembly->objects, constant, &object, sizeof(NT_OBJECT *)) ==
                        sizeof(NT_DELEGATE *);
    assert(result);
    return object;
}
