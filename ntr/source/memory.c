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
#include <malloc.h>
#include <netuno/memory.h>
#include <stdint.h>
#include <string.h>

// #define DEBUG_MEM

#ifdef DEBUG_MEM
typedef struct
{
    size_t size;
    uint64_t data[];
} Mem;
#endif

void *ntMalloc(size_t size)
{
#ifdef DEBUG_MEM
    Mem *mem = (Mem *)malloc(sizeof(Mem) + size);
    mem->size = size;
    return mem->data;
#else
    return malloc(size);
#endif
}

void ntFree(void *old)
{
#ifdef DEBUG_MEM
    if (old)
    {
        Mem *mem = (Mem *)(((uint8_t *)old) - offsetof(Mem, data));
        free(mem);
    }
#else
    free(old);
#endif
}

void *ntRealloc(void *old, size_t size)
{
#ifdef DEBUG_MEM
    if (old)
    {
        Mem *mem = (Mem *)(((uint8_t *)old) - offsetof(Mem, data));
        if (mem->size == size)
            return old;

        printf("Old: %p, Size: %d\n", old, mem->size);

        void *new = ntMalloc(size);
        printf("New: %p, Size: %d\n", new, size);
        if (size < mem->size)
            memcpy(new, old, size);
        else
            memcpy(new, old, mem->size);

        ntFree(old);
        return new;
    }
    else
        return ntMalloc(size);
#else
    return realloc(old, size);
#endif
}

void ntMemcpy(void *dst, const void *src, size_t size)
{
#ifdef DEBUG_MEM
    printf("Memcpy from %p to %p, Size: %d\n", src, dst, size);
#endif
    memcpy(dst, src, size);
}
