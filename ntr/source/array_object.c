#include "netuno/native.h"
#include <assert.h>
#include <netuno/array.h>
#include <netuno/array_object.h>
#include <netuno/delegate.h>
#include <netuno/memory.h>
#include <netuno/module.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/symbol.h>
#include <netuno/type.h>
#include <netuno/vm.h>
#include <stdbool.h>

static bool refEquals(NT_OBJECT *obj1, NT_OBJECT *obj2)
{
    return obj1 == obj2;
}

static void freeArrayObject(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_ARRAY);
    NT_ARRAY_OBJECT *array = (NT_ARRAY_OBJECT *)object;
    ntFreeArray(&array->array);
}

static const NT_STRING *arrayObjectToString(NT_OBJECT *object)
{
    assert(object->type->objectType == NT_OBJECT_ARRAY);
    const NT_ARRAY_OBJECT *array = (NT_ARRAY_OBJECT *)object;

    char_t *str;
    size_t length;
    {
        // get T type
        assert(array->object.type->genericTypeArgumentCount == 1);
        const NT_TYPE *t = array->object.type->genericTypeArguments[0];

        const size_t itSize = t->stackSize;

        // buffer for string
        NT_ARRAY buffer;
        ntInitArray(&buffer);
        ntArrayAdd(&buffer, U"[", sizeof(char_t));

        char temp[16];
        // ensure temp has enough size
        assert(sizeof(temp) >= itSize);

        for (size_t ioffset = 0; ioffset < array->array.count; ioffset += itSize)
        {
            const bool result = ntArrayGet(&array->array, ioffset, temp, itSize) == itSize;
            assert(result);

            const NT_STRING *valuestr = t->string((NT_OBJECT *)temp);
            ntArrayAdd(&buffer, valuestr->chars, valuestr->length * sizeof(char_t));
            ntFreeObject((NT_OBJECT *)valuestr);

            if (ioffset + itSize < array->array.count)
                ntArrayAdd(&buffer, U", ", sizeof(char_t) * 2);
        }

        ntArrayAdd(&buffer, U"]\0", sizeof(char_t) * 2);

        str = ntRealloc(buffer.data, buffer.count);
        length = (buffer.count / sizeof(char_t)) - 1;
    }

    return ntTakeString(str, length);
}

#define DEFINE_GET(__type, __ntType)                                                               \
    bool array_get_##__type(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType)                       \
    {                                                                                              \
        assert(vm);                                                                                \
        assert(delegateType);                                                                      \
        /* check delegateType has two arguments */                                                 \
        assert(delegateType->paramCount == 2);                                                     \
                                                                                                   \
        /* check first argument is array */                                                        \
        assert(delegateType->params[0].type);                                                      \
        assert(delegateType->params[0].type->objectType == NT_OBJECT_ARRAY);                       \
        if (delegateType->params[0].type == NULL ||                                                \
            delegateType->params[0].type->objectType != NT_OBJECT_ARRAY)                           \
            return false;                                                                          \
                                                                                                   \
        /* check second argument is a uint*/                                                       \
        assert(delegateType->params[1].type != (__ntType));                                        \
        if (delegateType->params[1].type != (__ntType))                                            \
            return false;                                                                          \
                                                                                                   \
        const NT_TYPE *arrayType = delegateType->params[0].type;                                   \
                                                                                                   \
        /* check if array type has generic type*/                                                  \
        assert(arrayType->genericTypeArgumentCount == 1);                                          \
        assert(arrayType->genericTypeArguments[0]);                                                \
        if (arrayType->genericTypeArgumentCount != 1 ||                                            \
            arrayType->genericTypeArguments[0] == NULL)                                            \
            return false;                                                                          \
                                                                                                   \
        const NT_TYPE *t = arrayType->genericTypeArguments[0];                                     \
                                                                                                   \
        /* check return value is of generic type t */                                              \
        assert(delegateType->returnType == t);                                                     \
        if (delegateType->returnType != t)                                                         \
            return false;                                                                          \
                                                                                                   \
        /* pop index */                                                                            \
        __type index;                                                                              \
                                                                                                   \
        bool result;                                                                               \
        switch (sizeof(__type))                                                                    \
        {                                                                                          \
        case sizeof(uint32_t):                                                                     \
            result = ntPop32(vm, (uint32_t *)&index);                                              \
            break;                                                                                 \
        case sizeof(uint64_t):                                                                     \
            result = ntPop64(vm, (uint64_t *)&index);                                              \
            break;                                                                                 \
        default:                                                                                   \
            assert(false);                                                                         \
            return false;                                                                          \
        }                                                                                          \
        assert(result);                                                                            \
        if (!result)                                                                               \
            return false;                                                                          \
                                                                                                   \
        /* pop array */                                                                            \
        NT_ARRAY_OBJECT *array;                                                                    \
        result = ntPopRef(vm, (NT_REF *)&array);                                                   \
        assert(result);                                                                            \
        assert(IS_VALID_OBJECT(array));                                                            \
        assert(ntTypeIsAssignableFrom(arrayType, array->object.type));                             \
        if (!result || !IS_VALID_OBJECT(array) ||                                                  \
            ntTypeIsAssignableFrom(arrayType, array->object.type))                                 \
            return false;                                                                          \
                                                                                                   \
        uint64_t temp;                                                                             \
        result = ntArrayGet(&array->array, (size_t)(index * t->stackSize), &temp, t->stackSize) == \
                 t->stackSize;                                                                     \
        assert(result);                                                                            \
        if (!result)                                                                               \
            return false;                                                                          \
                                                                                                   \
        /* push result in stack */                                                                 \
        switch (t->stackSize)                                                                      \
        {                                                                                          \
        case sizeof(uint32_t):                                                                     \
            ntPush32(vm, *(uint32_t *)&temp);                                                      \
            break;                                                                                 \
        case sizeof(uint64_t):                                                                     \
            ntPush64(vm, temp);                                                                    \
            break;                                                                                 \
        default:                                                                                   \
            assert(false);                                                                         \
            return false;                                                                          \
        }                                                                                          \
                                                                                                   \
        return true;                                                                               \
    }

#define DEFINE_SET(__type, __ntType)                                                               \
    bool array_set_##__type(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType)                       \
    {                                                                                              \
        assert(vm);                                                                                \
        assert(delegateType);                                                                      \
        /* check delegateType has two arguments */                                                 \
        assert(delegateType->paramCount == 3);                                                     \
                                                                                                   \
        /* check first argument is array */                                                        \
        assert(delegateType->params[0].type);                                                      \
        assert(delegateType->params[0].type->objectType == NT_OBJECT_ARRAY);                       \
        if (delegateType->params[0].type == NULL ||                                                \
            delegateType->params[0].type->objectType != NT_OBJECT_ARRAY)                           \
            return false;                                                                          \
                                                                                                   \
        /* check second argument is a uint*/                                                       \
        assert(delegateType->params[1].type != (__ntType));                                        \
        if (delegateType->params[1].type != (__ntType))                                            \
            return false;                                                                          \
                                                                                                   \
        const NT_TYPE *arrayType = delegateType->params[0].type;                                   \
                                                                                                   \
        /* check if array type has generic type*/                                                  \
        assert(arrayType->genericTypeArgumentCount == 1);                                          \
        assert(arrayType->genericTypeArguments[0]);                                                \
        if (arrayType->genericTypeArgumentCount != 1 ||                                            \
            arrayType->genericTypeArguments[0] == NULL)                                            \
            return false;                                                                          \
                                                                                                   \
        const NT_TYPE *t = arrayType->genericTypeArguments[0];                                     \
                                                                                                   \
        /* check third argument is a t */                                                          \
        assert(delegateType->params[2].type == t);                                                 \
        if (delegateType->params[2].type != t)                                                     \
            return false;                                                                          \
                                                                                                   \
        bool result;                                                                               \
        uint64_t value;                                                                            \
        /* pop value */                                                                            \
        switch (t->stackSize)                                                                      \
        {                                                                                          \
        case sizeof(uint32_t):                                                                     \
            ntPop32(vm, (uint32_t *)&value);                                                       \
            break;                                                                                 \
        case sizeof(uint64_t):                                                                     \
            ntPop64(vm, &value);                                                                   \
            break;                                                                                 \
        default:                                                                                   \
            assert(false);                                                                         \
            return false;                                                                          \
        }                                                                                          \
                                                                                                   \
        /* pop index */                                                                            \
        __type index;                                                                              \
        switch (sizeof(__type))                                                                    \
        {                                                                                          \
        case sizeof(uint32_t):                                                                     \
            result = ntPop32(vm, (uint32_t *)&index);                                              \
            break;                                                                                 \
        case sizeof(uint64_t):                                                                     \
            result = ntPop64(vm, (uint64_t *)&index);                                              \
            break;                                                                                 \
        default:                                                                                   \
            assert(false);                                                                         \
            return false;                                                                          \
        }                                                                                          \
        assert(result);                                                                            \
        if (!result)                                                                               \
            return false;                                                                          \
                                                                                                   \
        /* pop array */                                                                            \
        NT_ARRAY_OBJECT *array;                                                                    \
        result = ntPopRef(vm, (NT_REF *)&array);                                                   \
        assert(result);                                                                            \
        assert(IS_VALID_OBJECT(array));                                                            \
        assert(ntTypeIsAssignableFrom(arrayType, array->object.type));                             \
        if (!result || !IS_VALID_OBJECT(array) ||                                                  \
            ntTypeIsAssignableFrom(arrayType, array->object.type))                                 \
            return false;                                                                          \
                                                                                                   \
        ntArraySet(&array->array, (size_t)(index * t->stackSize), &value, t->stackSize);           \
                                                                                                   \
        return true;                                                                               \
    }

static NT_MODULE ARRAY = {
    .type.object =
        {
            .type = NULL,
        },
};

const NT_MODULE *ntArrayModule(void)
{
    if (ARRAY.type.object.type == NULL)
    {
        ntInitModule(&ARRAY);
        ntMakeConstant((NT_OBJECT *)&ARRAY);
        const char_t *moduleName = U"array";
        ARRAY.type.typeName = ntCopyString(moduleName, ntStrLen(moduleName));
        ntInitSymbolTable(&ARRAY.type.fields, (NT_SYMBOL_TABLE *)&ntType()->fields, STT_TYPE, NULL);
    }

    return &ARRAY;
}

DEFINE_GET(uint32_t, ntU32Type())
DEFINE_SET(uint32_t, ntU32Type())

static const NT_TYPE *ntCreateArrayType(const NT_STRING *arrayTypeName, const NT_TYPE *t)
{
    NT_TYPE *arrayType = (NT_TYPE *)ntCreateObject(ntType());

    arrayType->objectType = NT_OBJECT_ARRAY;
    arrayType->typeName = arrayTypeName;
    arrayType->free = freeArrayObject;
    arrayType->string = arrayObjectToString;
    arrayType->equals = refEquals;
    arrayType->stackSize = sizeof(NT_REF);
    arrayType->instanceSize = sizeof(NT_ARRAY_OBJECT);
    arrayType->baseType = ntObjectType();

    NT_MODULE *module = (NT_MODULE *)ntArrayModule();

    ntInitSymbolTable(&arrayType->fields, NULL, STT_TYPE, NULL);
    arrayType->genericTypeArgumentCount = 1;
    arrayType->genericTypeArguments = (const NT_TYPE **)ntMalloc(sizeof(const NT_TYPE *));
    arrayType->genericTypeArguments[0] = t;

    const NT_STRING *arrayName = ntCopyString(U"array", 5);
    const NT_STRING *indexName = ntCopyString(U"index", 5);
    const NT_STRING *valueName = ntCopyString(U"value", 5);

    NT_PARAM params[3] = {{.type = arrayType, .name = arrayName},
                          {.type = ntU32Type(), .name = indexName}};

    ntCreateNativeFunction(module, &arrayType->fields, U"get", t, 2, params, array_get_uint32_t,
                           true);

    params[2] = (NT_PARAM){.type = t, .name = valueName};
    ntCreateNativeFunction(module, &arrayType->fields, U"set", ntVoidType(), 3, params,
                           array_set_uint32_t, true);

    return arrayType;
}

const NT_STRING *ntArrayTypeName(const NT_TYPE *t)
{
    assert(t);

    char_t *str;
    size_t length;
    {
        NT_ARRAY array;
        ntInitArray(&array);

        const char_t *const arrayPrefix = U"array(";
        ntArrayAdd(&array, arrayPrefix, sizeof(char_t) * ntStrLen(arrayPrefix));

        // add T name
        const NT_STRING *tName = t->typeName;
        ntArrayAdd(&array, tName->chars, tName->length * sizeof(char_t));

        ntArrayAdd(&array, U")\0", sizeof(char_t) * 2);

        str = ntRealloc(array.data, array.count);
        length = (array.count / sizeof(char_t)) - 1;
    }

    return ntTakeString(str, length);
}

static NT_TABLE arrayTypeTable = {.count = 0, .size = 0, .pEntries = NULL};

const NT_TYPE *ntArrayType(const NT_TYPE *t)
{
    assert(t);
    const NT_STRING *name = ntArrayTypeName(t);

    const NT_TYPE *arrayType = NULL;
    if (ntTableGet(&arrayTypeTable, name, (void **)&arrayType))
        return arrayType;

    arrayType = ntCreateArrayType(name, t);
    const bool result = ntTableSet(&arrayTypeTable, name, (void *)arrayType);
    assert(result);
    return arrayType;
}
