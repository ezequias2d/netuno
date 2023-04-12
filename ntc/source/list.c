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

bool ntListPop(NT_LIST list, void **pValue)
{
    assert(list);
    LIST *l = (LIST *)list;
    if (l->count == 0)
        return false;

    const size_t last = --(l->count);
    void *result = l->elements[last];
    l->elements[last] = NULL;
    if (pValue)
        *pValue = result;
    return true;
}

bool ntListPeek(NT_LIST list, size_t index, void **pValue)
{
    assert(list);
    LIST *l = (LIST *)list;
    if (l->count == 0)
        return false;

    const size_t last = l->count - 1;
    if (pValue)
        *pValue = l->elements[last - index];
    return true;
}

bool ntListGet(NT_LIST list, size_t index, void **pValue)
{
    assert(list);
    LIST *l = (LIST *)list;
    if (index >= l->count)
        return false;

    if (pValue)
        *pValue = ((LIST *)list)->elements[index];

    return true;
}
