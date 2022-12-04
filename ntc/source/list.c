#include "list.h"
#include <assert.h>
#include <netuno/memory.h>

typedef struct
{
    size_t size;
    size_t count;
    void **elements;
} LIST;

NT_LIST ntCreateList(void)
{
    LIST *list = (LIST *)ntMalloc(sizeof(LIST));
    list->size = 0;
    list->count = 0;
    list->elements = NULL;
    return list;
}

size_t ntListLen(NT_LIST list)
{
    assert(list);
    return ((LIST *)list)->count;
}

void ntFreeList(NT_LIST list)
{
    assert(list);
    LIST *l = (LIST *)list;
    ntFree(l->elements);
    ntFree(l);
}

void ntListAdd(NT_LIST list, void *value)
{
    assert(list);
    LIST *l = (LIST *)list;
    if (l->count >= l->size)
    {
        size_t newSize = l->size * 3 / 2;
        if (newSize <= 0)
            newSize = 2;
        l->elements = ntRealloc(l->elements, sizeof(void *) * newSize);
        l->size = newSize;
    }
    l->elements[l->count++] = value;
}

void ntListPush(NT_LIST list, void *value)
{
    ntListAdd(list, value);
}

void *ntListPop(NT_LIST list)
{
    assert(list);
    LIST *l = (LIST *)list;
    const size_t last = --(l->count);
    void *result = l->elements[last];
    l->elements[last] = NULL;
    return result;
}

void *ntListGet(NT_LIST list, size_t index)
{
    assert(list);
    return ((LIST *)list)->elements[index];
}
