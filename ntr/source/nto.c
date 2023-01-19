#include <assert.h>
#include <netuno/array.h>
#include <netuno/memory.h>
#include <netuno/nto.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <netuno/type.h>
#include <netuno/varint.h>

NT_ASSEMBLY *ntCreateAssembly(void)
{
    NT_ASSEMBLY *assembly = (NT_ASSEMBLY *)ntMalloc(sizeof(NT_ASSEMBLY));
    ntInitArray(&assembly->chunks);
    ntInitArray(&assembly->types);
    ntInitTable(&assembly->table);
    ntInitTable(&assembly->strings);
    ntInitTable(&assembly->delegates);
    return assembly;
}

static void freeDelegate(const NT_STRING *key, void *data, void *userdata)
{
    assert(userdata == NULL);
    assert(data);

    ntFreeObject((NT_OBJECT *)key);
    ntFree(data);
}

void ntFreeAssembly(NT_ASSEMBLY *assembly)
{
    for (size_t i = 0; i < assembly->chunks.count / sizeof(NT_CHUNK *); ++i)
    {
        NT_CHUNK *chunk;
        ntArrayGet(&assembly->chunks, i * sizeof(NT_CHUNK *), &chunk, sizeof(NT_CHUNK *));
        ntFreeChunk(chunk);
    }

    ntTableForAll(&assembly->delegates, freeDelegate, NULL);
    ntDeinitTable(&assembly->delegates);

    ntDeinitArray(&assembly->chunks);
    ntDeinitArray(&assembly->types);

    ntDeinitTable(&assembly->table);
    ntDeinitTable(&assembly->strings);

    ntFree(assembly);
}

const NT_DELEGATE_TYPE *ntDelegateType(NT_ASSEMBLY *assembly, const NT_TYPE *returnType,
                                       size_t paramCount, const NT_PARAM *params)
{
    assert(assembly);

    const NT_STRING *delegateName;
    {
        char_t *name = ntDelegateTypeName(returnType, paramCount, params);
        const size_t len = ntStrLen(name);
        delegateName = ntTakeString(name, len);
    }

    const NT_DELEGATE_TYPE *delegateType = NULL;
    if (ntTableGet(&assembly->delegates, delegateName, (void **)&delegateType))
        return delegateType;

    delegateType = ntCreateDelegateType(delegateName, returnType, paramCount, params);
    ntTableSet(&assembly->delegates, delegateName, (void *)delegateType);
    return delegateType;
}

NT_CHUNK *ntCreateChunk(void)
{
    NT_CHUNK *chunk = (NT_CHUNK *)ntMalloc(sizeof(NT_CHUNK));
    ntInitChunk(chunk);
    return chunk;
}

void ntInitChunk(NT_CHUNK *chunk)
{
    ntInitArray(&chunk->code);
    ntInitArray(&chunk->lines);
    ntInitArray(&chunk->constants);
}

void ntDeinitChunk(NT_CHUNK *chunk)
{
    ntDeinitArray(&chunk->code);
    ntDeinitArray(&chunk->lines);
    ntDeinitArray(&chunk->constants);
}

void ntFreeChunk(NT_CHUNK *chunk)
{
    ntDeinitArray(&chunk->code);
    ntDeinitArray(&chunk->lines);
    ntDeinitArray(&chunk->constants);
    ntFree(chunk);
}

static void addLine(NT_CHUNK *chunk, const size_t offset, const size_t line)
{
    bool addLine = false;
    if (chunk->lines.count < sizeof(NT_LINE))
    {
        addLine = true;
    }
    else
    {
        NT_LINE l;
        ntArrayGet(&chunk->lines, chunk->lines.count - sizeof(NT_LINE), &l, sizeof(NT_LINE));
        if (l.line != line)
            addLine = true;
    }

    if (addLine)
    {
        NT_LINE l;
        l.start = offset;
        l.line = line;
        ntArrayAdd(&chunk->lines, &l, sizeof(NT_LINE));
    }
}

int64_t ntGetLine(const NT_CHUNK *chunk, const size_t offset, bool *atStart)
{
    NT_LINE result = {.start = 0, .line = -1};
    for (size_t i = 0; i < chunk->lines.count; i += sizeof(NT_LINE))
    {
        NT_LINE current;
        ntArrayGet(&chunk->lines, i, &current, sizeof(NT_LINE));
        if (current.start > offset)
            break;
        result = current;
    }
    *atStart = (result.start == offset);
    return result.line;
}

size_t ntWriteChunk(NT_CHUNK *chunk, const uint8_t value, const int64_t line)
{
    size_t offset = ntArrayAdd(&chunk->code, &value, sizeof(uint8_t));
    addLine(chunk, offset, line);
    return offset;
}

void ntInsertChunk(NT_CHUNK *chunk, const size_t offset, const void *data, const size_t length)
{
    ntArrayInsert(&chunk->code, offset, data, length);
}

void ntInsertChunkVarint(NT_CHUNK *chunk, const size_t offset, const uint64_t value)
{
    ntArrayInsertVarint(&chunk->code, offset, value);
}

size_t ntWriteChunkVarint(NT_CHUNK *chunk, const uint64_t value, const int64_t line)
{
    size_t offset = ntArrayAddVarint(&chunk->code, value);
    addLine(chunk, offset, line);
    return offset;
}

uint8_t ntRead(const NT_CHUNK *chunk, const size_t offset)
{
    uint8_t value;
    const bool result =
        ntArrayGet(&chunk->code, offset, &value, sizeof(uint8_t)) == sizeof(uint8_t);
    assert(result);
    return value;
}

size_t ntReadVariant(const NT_CHUNK *chunk, const size_t offset, uint64_t *value)
{
    return ntArrayGetVarint(&chunk->code, offset, value);
}

uint64_t ntAddConstant32(NT_CHUNK *chunk, const uint32_t value)
{
    size_t offset;
    if (ntArrayFind(&chunk->constants, &value, sizeof(value), &offset))
        return offset;

    ntArrayAddU32(&chunk->constants, value);
    return chunk->constants.count - sizeof(value);
}

uint64_t ntAddConstant64(NT_CHUNK *chunk, const uint64_t value)
{
    size_t offset;
    if (ntArrayFind(&chunk->constants, &value, sizeof(value), &offset))
        return offset;

    ntArrayAddU64(&chunk->constants, value);
    return chunk->constants.count - sizeof(value);
}

uint64_t ntAddConstantString(NT_CHUNK *chunk, const char_t *str, const size_t length)
{
    size_t offset;

    const size_t nullTerminatedSize = sizeof(char_t) * (length + 1);
    char_t *nullTerminatedStr = (char_t *)ntMalloc(nullTerminatedSize);
    ntMemcpy(nullTerminatedStr, str, length * sizeof(char_t));
    nullTerminatedStr[length] = '\0';

    if (ntArrayFind(&chunk->constants, str, nullTerminatedSize, &offset))
    {
        ntFree(nullTerminatedStr);
        return offset;
    }
    ntArrayAdd(&chunk->constants, nullTerminatedStr, nullTerminatedSize);
    ntFree(nullTerminatedStr);
    return chunk->constants.count - nullTerminatedSize;
}

static uint64_t addConstantType(NT_CHUNK *chunk, const NT_TYPE *type)
{
    uint64_t offset;

    const size_t size = sizeof(NT_LOCAL_TYPE);
    const NT_LOCAL_TYPE localType = {
        .objectType = type->objectType,
        .constantTypeName =
            ntAddConstantString(chunk, type->typeName->chars, type->typeName->length),
        .stackSize = type->stackSize,
        .instanceSize = type->instanceSize,
    };

    if (ntArrayFind(&chunk->constants, &localType, size, &offset))
        return offset;

    ntArrayAdd(&chunk->constants, &localType, size);
    return chunk->constants.count - size;
}

typedef struct
{
    NT_CHUNK *chunk;
    NT_LOCAL_CUSTOM_TYPE *localType;
} ADD_CUSTOM_TYPE_USERDATA;

static void addCustomTypeFieldIt(const NT_STRING *key, void *value, void *userdata)
{
    ADD_CUSTOM_TYPE_USERDATA *const u = (ADD_CUSTOM_TYPE_USERDATA *)userdata;
    NT_LOCAL_CUSTOM_TYPE *const localType = u->localType;
    NT_CHUNK *chunk = u->chunk;

    const NT_FIELD *const field = (const NT_FIELD *)value;

    const uint64_t i = localType->fieldsCount++;

    localType->fields[i] = (NT_LOCAL_FIELD){
        .constantType = ntAddConstantType(chunk, field->fieldType),
        .constantName = ntAddConstantString(chunk, key->chars, key->length),
        .fieldOffset = field->offset,
    };
}

static uint64_t addConstantCustomType(NT_CHUNK *chunk, const NT_CUSTOM_TYPE *type)
{
    uint64_t offset;

    const size_t size = sizeof(NT_LOCAL_CUSTOM_TYPE) + type->fields.count;
    NT_LOCAL_CUSTOM_TYPE *localType = (NT_LOCAL_CUSTOM_TYPE *)ntMalloc(size);
    *localType = (NT_LOCAL_CUSTOM_TYPE){
        .localType =
            {
                .objectType = type->type.objectType,
                .constantTypeName = ntAddConstantString(chunk, type->type.typeName->chars,
                                                        type->type.typeName->length),
                .stackSize = type->type.stackSize,
                .instanceSize = type->type.instanceSize,
            },
        .constantFreeFunction = ntAddConstantFunction(
            chunk, type->free->addr, (NT_DELEGATE_TYPE *)type->free->object.type, type->free->name),
        .constantStringFunction = ntAddConstantFunction(
            chunk, type->string->addr, (NT_DELEGATE_TYPE *)type->string->object.type,
            type->string->name),
        .constantEqualsFunction = ntAddConstantFunction(
            chunk, type->equals->addr, (NT_DELEGATE_TYPE *)type->equals->object.type,
            type->equals->name),
        .fieldsCount = 0,
    };
    // type->fields.count

    ADD_CUSTOM_TYPE_USERDATA userdata = {.localType = localType, .chunk = chunk};
    ntTableForAll(&type->fields, addCustomTypeFieldIt, &userdata);

    if (ntArrayFind(&chunk->constants, localType, size, &offset))
        return offset;

    ntArrayAdd(&chunk->constants, localType, size);
    return chunk->constants.count - size;
}

static uint64_t addConstantDelegateType(NT_CHUNK *chunk, const NT_DELEGATE_TYPE *delegateType)
{
    size_t offset;

    const size_t size =
        sizeof(NT_LOCAL_DELEGATE_TYPE) + sizeof(uint64_t) * delegateType->paramCount;

    NT_LOCAL_DELEGATE_TYPE *localType = (NT_LOCAL_DELEGATE_TYPE *)ntMalloc(size);

    localType->paramCount = delegateType->paramCount;
    localType->constantReturnType = ntAddConstantType(chunk, delegateType->returnType);

    for (uint64_t i = 0; i < delegateType->paramCount; ++i)
    {
        const NT_LOCAL_PARAM param = {
            .constantType = ntAddConstantType(chunk, delegateType->params[i].type),
            .constantName = ntAddConstantString(chunk, delegateType->params[i].name->chars,
                                                delegateType->params[i].name->length),
        };
        localType->constantParams[i] = param;
    }

    if (ntArrayFind(&chunk->constants, localType, size, &offset))
    {
        ntFree(localType);
        return offset;
    }

    ntArrayAdd(&chunk->constants, localType, size);
    ntFree(localType);
    return chunk->constants.count - size;
}

uint64_t ntAddConstantType(NT_CHUNK *chunk, const NT_TYPE *type)
{

    if (type->objectType == NT_OBJECT_CUSTOM)
        return addConstantCustomType(chunk, (const NT_CUSTOM_TYPE *)type);
    else if (type->objectType == NT_OBJECT_FUNCTION)
        return addConstantDelegateType(chunk, (const NT_DELEGATE_TYPE *)type);

    return addConstantType(chunk, type);
}

uint64_t ntAddConstantFunction(NT_CHUNK *chunk, size_t addr, const NT_DELEGATE_TYPE *delegateType,
                               const NT_STRING *name)
{
    uint64_t offset;

    assert(chunk);
    assert(delegateType);
    assert(delegateType->type.objectType == NT_OBJECT_FUNCTION);
    assert(name);
    assert(name->object.type->objectType == NT_OBJECT_STRING);

    const NT_LOCAL_FUNCTION function = {
        .addr = addr,
        .constantDelegateType = ntAddConstantType(chunk, (const NT_TYPE *)delegateType),
        .constantString = ntAddConstantString(chunk, name->chars, name->length),
    };

    if (ntArrayFind(&chunk->constants, &function, sizeof(NT_LOCAL_FUNCTION), &offset))
        return offset;

    ntArrayAdd(&chunk->constants, &function, sizeof(NT_LOCAL_FUNCTION));
    return chunk->constants.count - sizeof(NT_LOCAL_FUNCTION);
}
