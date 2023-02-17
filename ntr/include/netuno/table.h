#ifndef NT_TABLE_H
#define NT_TABLE_H

#include <netuno/common.h>

typedef struct _NT_STRING NT_STRING;
typedef struct _NT_ENTRY NT_ENTRY;
typedef struct _NT_TABLE NT_TABLE;

struct _NT_ENTRY
{
    const NT_STRING *key;
    void *value;
};

struct _NT_TABLE
{
    size_t count;
    size_t size;
    NT_ENTRY *pEntries;
};

typedef void (*tableCallback)(const NT_STRING *key, void *value, void *userdata);

NT_TABLE *ntCreateTable(void);
void ntInitTable(NT_TABLE *table);
void ntDeinitTable(NT_TABLE *table);
void ntFreeTable(NT_TABLE *table);
void ntTableForAll(const NT_TABLE *table, tableCallback callback, void *userdata);
bool ntTableSet(NT_TABLE *table, const NT_STRING *key, void *value);
void ntTableAddAll(const NT_TABLE *from, NT_TABLE *to);
bool ntTableGet(const NT_TABLE *table, const NT_STRING *key, void **value);
bool ntTableDelete(NT_TABLE *table, const NT_STRING *key, void **value);
const NT_STRING *ntTableFindString(const NT_TABLE *table, const char_t *chars, size_t length,
                                   uint32_t hash);

#endif
