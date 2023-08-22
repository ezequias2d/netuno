#include "plist.h"
#include <assert.h>
#include <netuno/memory.h>

void listInit(NIL_LIST *list)
{
    assert(list);
    list->count = 0;
    list->size = 0;
    list->elems = NULL;
}

void listDeinit(NIL_LIST *list)
{
    assert(list);
    assert(list->count <= list->size);
    if (list->size == 0)
        assert(list->elems == NULL);

    ntFree(list->elems);

    list->count = 0;
    list->size = 0;
    list->elems = NULL;
}

static void grow(NIL_LIST *list, size_t need)
{
    assert(list->count <= list->size);
    if (list->size - list->count < need)
    {
        size_t newSize = list->size * 3 / 2;
        if (newSize < list->count + need)
            newSize = list->count + need;

        list->elems = ntRealloc(list->elems, sizeof(void *) * newSize);
        assert(list->elems);
        list->size = newSize;
    }
}

#include <stdio.h>
void listAdd(NIL_LIST *list, void *value)
{
    assert(list);
    assert(list->count <= list->size);
    assert(value);

    size_t p = (size_t)(list->elems + list->count);
    if (p == 4287632)
    {
        int a = 1;
        printf("\n-------------6: %zu\n", p);
        assert(a);
    }
    grow(list, 1);
    list->elems[list->count++] = value;
}

void listRemove(NIL_LIST *list, size_t index)
{
    assert(list);
    assert(list->count > 0);
    assert(index < list->count);

    const size_t last = list->count - 1;
    for (size_t i = index; i < last; ++i)
    {
        list->elems[i] = list->elems[i + 1];
    }
    list->elems[last] = NULL;
}
