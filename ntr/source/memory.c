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
