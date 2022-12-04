#include <assert.h>
#include <netuno/debug.h>
#include <netuno/memory.h>
#include <netuno/nto.h>
#include <netuno/str.h>
#include <stdio.h>

static const char *const labels[] = {
#define bytecode(a) #a,
#include <netuno/opcode.inc>
#undef bytecode
};

void ntDisassembleChunk(const NT_CHUNK *chunk, const char *name)
{
    printf("====== %s =====\n", name);

    for (size_t i = 0; i < chunk->code.count;)
        i = ntDisassembleInstruction(chunk, i);
}

static size_t simpleInstruction(const char *name, const size_t offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static size_t constant32Instruction(const char *name, const NT_CHUNK *chunk, const size_t offset)
{
    uint64_t constant;
    const size_t readed = ntReadVariant(chunk, offset + 1, &constant);
    printf("%-16s %4ld '", name, constant);

    uint32_t value;
    assert(ntArrayGetU32(&chunk->constants, constant, &value));
    printf("%4d\n", value);

    return readed + 1;
}

static size_t constant64Instruction(const char *name, const NT_CHUNK *chunk, const size_t offset)
{
    uint64_t constant;
    const size_t readed = ntReadVariant(chunk, offset + 1, &constant);
    printf("%-16s %4ld '", name, constant);

    uint64_t value;
    assert(ntArrayGetU64(&chunk->constants, constant, &value));
    printf("%ld\n", value);

    return readed + 1;
}

static size_t constantStringInstruction(const char *name, const NT_CHUNK *chunk,
                                        const size_t offset)
{
    uint64_t constant;
    const size_t readed = ntReadVariant(chunk, offset + 1, &constant);
    printf("%-16s %4ld '", name, constant);

    size_t size;
    ntArrayGetString(&chunk->constants, constant, NULL, &size);
    char_t *strt = ntMalloc(sizeof(char_t) * (size + 1));
    ntArrayGetString(&chunk->constants, constant, strt, &size);
    strt[size] = '\0';

    char *str = ntToChar(strt);
    ntFree(strt);

    printf("%s\n", str);

    ntFree(str);

    return readed + 1;
}

size_t ntDisassembleInstruction(const NT_CHUNK *chunk, const size_t offset)
{
    printf("%04ld ", offset);

    bool atStart;
    int64_t line = ntGetLine(chunk, offset, &atStart);

    if (atStart)
        printf("%4ld ", line);
    else
        printf("   | ");

    uint8_t instruction;
    assert(ntArrayGet(&chunk->code, offset, &instruction, sizeof(uint8_t)));

    if (instruction < BC_LAST)
    {
        const char *label = labels[instruction];
        switch (instruction)
        {
        case BC_CONST_32:
            return constant32Instruction(label, chunk, offset);
        case BC_CONST_64:
            return constant64Instruction(label, chunk, offset);
        case BC_CONST_STRING:
            return constantStringInstruction(label, chunk, offset);
        default:
            return simpleInstruction(label, offset);
        }
    }
    else
    {
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}
