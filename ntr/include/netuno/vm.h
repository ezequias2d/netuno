#ifndef NT_NTR_H
#define NT_NTR_H

#include <netuno/assembly.h>

#ifndef NDEBUG
#define DEBUG_TRACE_EXECUTION
#endif

#define STACK_MAX 4096
#define CALL_STACK_MAX 4096

typedef enum
{
    NT_OK,
    NT_COMPILE_ERROR,
    NT_RUNTIME_ERROR,
    NT_STACK_OVERFLOW,
} NT_RESULT;

typedef struct _NT_VM
{
    const NT_MODULE *module;
    NT_ASSEMBLY *assembly;
    size_t pc;
    uint8_t *stack;
    uint8_t *stackTop;
    uint8_t *callStack;
    uint8_t *callStackTop;
    bool stackOverflow;
#ifdef DEBUG_TRACE_EXECUTION
    size_t *stackType;
    size_t *stackTypeTop;
#endif
} NT_VM;

NT_VM *ntCreateVM(void);
void ntFreeVM(NT_VM *vm);

NT_RESULT ntRun(NT_VM *vm, NT_ASSEMBLY *assembly, const NT_DELEGATE *entryPoint);

void ntResetStack(NT_VM *vm);
bool ntPush(NT_VM *vm, const void *data, const size_t dataSize);
bool ntPop(NT_VM *vm, void *data, const size_t dataSize);
bool ntPop32(NT_VM *vm, uint32_t *value);
bool ntPush32(NT_VM *vm, const uint32_t value);
bool ntPop64(NT_VM *vm, uint64_t *value);
bool ntPush64(NT_VM *vm, const uint64_t value);
#endif
