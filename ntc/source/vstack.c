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
#include "vstack.h"
#include <assert.h>
#include <netuno/memory.h>

NT_VSTACK *ntCreateVStack(void)
{
    NT_VSTACK *stack = (NT_VSTACK *)ntMalloc(sizeof(NT_VSTACK));
    stack->list = ntCreateList();
    stack->sp = 0;
    return stack;
}

void ntFreeVStack(NT_VSTACK *stack)
{
    ntFreeList(stack->list);
    ntFree(stack);
}

size_t ntVPush(NT_VSTACK *stack, const NT_TYPE *type)
{
    ntListPush(stack->list, (void *)type);
    stack->sp += type->stackSize;
    return stack->sp;
}

size_t ntVPop(NT_VSTACK *stack, const NT_TYPE **ppType)
{
    assert(ntListLen(stack->list) != 0);

    const NT_TYPE *pType = (const NT_TYPE *)ntListPop(stack->list);
    if (ppType != NULL)
        *ppType = pType;

    const size_t sp = stack->sp;
    stack->sp -= pType->stackSize;
    return sp;
}

void ntVPeek(NT_VSTACK *stack, const NT_TYPE **ppType, size_t index)
{
    assert(ntListLen(stack->list) != 0);

    const NT_TYPE *pType = (const NT_TYPE *)ntListPeek(stack->list, index);
    if (ppType != NULL)
        *ppType = pType;
}
