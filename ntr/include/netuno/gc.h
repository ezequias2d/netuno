#ifndef NETUNO_GC_H
#define NETUNO_GC_H

#include "array.h"
#include "object.h"
#include "table.h"

typedef struct _NT_GC
{
    NT_ARRAY objects;
} NT_GC;

NT_GC *ntCreateGarbageCollector(void);
void ntFreeGarbageCollector(NT_GC *gc);
uint32_t ntAddObject(NT_GC *gc, NT_OBJECT *object);
NT_OBJECT *ntGetObject(const NT_GC *gc, uint32_t id);
// void ntRefObject(NT_GC *gc, uint32_t id);
// void ntUnrefObject(NT_GC *gc, uint32_t id);

#endif
