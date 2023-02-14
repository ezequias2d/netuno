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
