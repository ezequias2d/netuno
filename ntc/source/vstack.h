#ifndef VSTACK_H
#define VSTACK_H

#include "list.h"
#include <netuno/type.h>

typedef struct _NT_VSTACK
{
    NT_LIST list;
    size_t sp;
} NT_VSTACK;

NT_VSTACK *ntCreateVStack(void);
void ntFreeVStack(NT_VSTACK *stack);
size_t ntVPush(NT_VSTACK *stack, const NT_TYPE *type);
size_t ntVPop(NT_VSTACK *stack, const NT_TYPE **type);
void ntVPeek(NT_VSTACK *stack, const NT_TYPE **ppType, size_t index);

#endif
