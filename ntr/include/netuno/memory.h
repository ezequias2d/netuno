#ifndef NT_MEMORY_H
#define NT_MEMORY_H

#include <stddef.h>

void *ntMalloc(size_t size);
void ntFree(void *);
void *ntRealloc(void *old, size_t size);
void ntMemcpy(void *dst, const void *src, size_t size);

#endif
