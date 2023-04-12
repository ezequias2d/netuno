#ifndef NIR_PLIST_H
#define NIR_PLIST_H

#include <netuno/common.h>

NT_HANDLE(NIR_LIST)

struct _NIR_LIST
{
    void **elems;
    size_t count;
    size_t size;
};

#define LIST(t, name, list)                                                    \
    union {                                                                    \
        t *name;                                                               \
        NIR_LIST list;                                                         \
    }

void listInit(NIR_LIST *list);
void listDeinit(NIR_LIST *list);
void listAdd(NIR_LIST *list, void *value);
void listRemove(NIR_LIST *list, size_t index);

#endif
