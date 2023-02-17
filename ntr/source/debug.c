#include <assert.h>
#include <netuno/debug.h>
#include <netuno/memory.h>
#include <netuno/module.h>
#include <netuno/opcode.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <stdio.h>

static const char *const labels[] = {
#define bytecode(a) #a,
#include <netuno/opcode.inc>
#undef bytecode
};

void ntDisassembleModule(const NT_ASSEMBLY *assembly, const NT_MODULE *module, const char *name)
{
    printf("====== %s =====\n", name);

    for (size_t i = 0; i < module->code.count; ++i)
        i = ntDisassembleInstruction(assembly, module, i);
}

static size_t simpleInstruction(const char *name, const size_t offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static size_t constant32Instruction(const char *name, const NT_MODULE *module, const size_t offset)
{
    uint64_t constant;
    const size_t readed = ntReadVariant(module, offset + 1, &constant);
    printf("%-16s %4ld '", name, constant);

    uint32_t value;
    const size_t readed2 = ntArrayGetU32(&module->constants, constant, &value);
    assert(readed2 == sizeof(uint32_t));
    printf("%4d\n", value);

    return readed + 1;
}

static size_t constant64Instruction(const char *name, const NT_MODULE *module, const size_t offset)
{
    uint64_t constant;
    const size_t readed = ntReadVariant(module, offset + 1, &constant);
    printf("%-16s %4ld '", name, constant);

    uint64_t value;
    const size_t size = ntArrayGetU64(&module->constants, constant, &value);
    assert(size);
    printf("%ld\n", value);

    return readed + 1;
}

static size_t popInstruction(const char *name, const NT_MODULE *module, const size_t offset)
{
    uint64_t size;
    const size_t readed = ntReadVariant(module, offset + 1, &size);
    printf("%-16s '%ld\n", name, size * sizeof(uint32_t));

    return readed + 1;
}

static size_t constantObjectInstruction(const char *name, const NT_ASSEMBLY *assembly,
                                        const NT_MODULE *module, const size_t offset)
{
    uint64_t constant;
    const size_t readed = ntReadVariant(module, offset + 1, &constant);
    printf("%-16s %4ld '", name, constant);

    NT_OBJECT *object = ntGetConstantObject(assembly, constant);
    const NT_STRING *string = ntToString(object);

    char *str = ntToCharFixed(string->chars, string->length);
    ntFreeObject((NT_OBJECT *)string);
    printf("%s\n", str);
    ntFree(str);

    return readed + 1;
}

size_t ntDisassembleInstruction(const NT_ASSEMBLY *assembly, const NT_MODULE *module,
                                const size_t offset)
{
    printf("%04ld ", offset);

    bool atStart;
    int64_t line = ntGetLine(module, offset, &atStart);

    if (atStart)
        printf("%4ld ", line);
    else
        printf("   | ");

    uint8_t instruction;
    const size_t size = ntArrayGet(&module->code, offset, &instruction, sizeof(uint8_t));
    assert(size);

    if (instruction < BC_LAST)
    {
        const char *label = labels[instruction];
        switch (instruction)
        {
        case BC_CONST_32:
            return constant32Instruction(label, module, offset);
        case BC_CONST_64:
            return constant64Instruction(label, module, offset);
        case BC_CONST_OBJECT:
            return constantObjectInstruction(label, assembly, module, offset);
        case BC_POP:
            return popInstruction(label, module, offset);
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
