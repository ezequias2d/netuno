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
bool ntPushRef(NT_VM *vm, NT_REF value);
bool ntPopRef(NT_VM *vm, NT_REF *value);
bool ntPop64(NT_VM *vm, uint64_t *value);
bool ntPush64(NT_VM *vm, const uint64_t value);
bool ntCall(NT_VM *vm, const NT_DELEGATE *delegate);

#endif
