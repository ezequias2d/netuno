#include <assert.h>
#include <netuno/memory.h>
#include <netuno/nto.h>
#include <netuno/str.h>
#include <netuno/varint.h>
#include <string.h>

NT_ASSEMBLY *ntCreateAssembly()
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

const NT_TYPE *ntDelegateType(NT_ASSEMBLY *assembly, const NT_TYPE *returnType, size_t paramCount,
                              const NT_PARAM *params)
{
    assert(assembly);

    const NT_STRING *delegateName;
    {
        char_t *name = ntDelegateTypeName(returnType, paramCount, params);
        const size_t len = ntStrLen(name);
        delegateName = ntTakeString(name, len);
    }

    NT_TYPE *delegateType = NULL;
    if (ntTableGet(&assembly->delegates, delegateName, (void **)&delegateType))
        return delegateType;

    delegateType = ntCreateDelegateType(delegateName, returnType, paramCount, params);
    ntTableSet(&assembly->delegates, delegateName, delegateType);
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

size_t ntWriteChunkVarintSigned(NT_CHUNK *chunk, const int64_t value, const int64_t line)
{
    size_t offset = ntArrayAddVarint(&chunk->code, ZigZagEncoding(value));
    addLine(chunk, offset, line);
    return offset;
}

uint8_t ntRead(const NT_CHUNK *chunk, const size_t offset)
{
    uint8_t value;
    assert(ntArrayGet(&chunk->code, offset, &value, sizeof(uint8_t)) == sizeof(uint8_t));
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
