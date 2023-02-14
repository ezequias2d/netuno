#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdint.h>

typedef void *NT_LIST;

NT_LIST ntCreateList(void);
size_t ntListLen(NT_LIST list);
void ntFreeList(NT_LIST list);
void ntListAdd(NT_LIST list, void *value);
void *ntListGet(NT_LIST list, size_t index);
void ntListPush(NT_LIST list, void *value);
void *ntListPop(NT_LIST list);
void *ntListPeek(NT_LIST list, size_t index);

#endif
