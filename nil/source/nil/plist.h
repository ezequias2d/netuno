#ifndef NIL_PLIST_H
#define NIL_PLIST_H

#include <netuno/common.h>

NT_HANDLE(NIL_LIST)

struct _NIL_LIST
{
    void **elems;
    size_t count;
    size_t size;
};

#define LIST(t, name, list)                                                    \
    union {                                                                    \
        t *name;                                                               \
        NIL_LIST list;                                                         \
    }

void listInit(NIL_LIST *list);
void listDeinit(NIL_LIST *list);
void listAdd(NIL_LIST *list, void *value);
void listRemove(NIL_LIST *list, size_t index);

#endif
