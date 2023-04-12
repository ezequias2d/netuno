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
#include "pstring.h"
#include <netuno/memory.h>
#include <netuno/string.h>
#include <netuno/table.h>
#include <string.h>

#define TABLE_MAX_LOAD 0.75
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

NT_TABLE *ntCreateTable(void)
{
    NT_TABLE *table = ntMalloc(sizeof(NT_TABLE));
    ntInitTable(table);
    return table;
}

void ntInitTable(NT_TABLE *table)
{
    table->count = 0;
    table->size = 0;
    table->pEntries = NULL;
}

void ntDeinitTable(NT_TABLE *table)
{
    table->count = 0;
    table->size = 0;
    if (table->pEntries)
        ntFree(table->pEntries);
}

void ntFreeTable(NT_TABLE *table)
{
    ntDeinitTable(table);
    ntFree(table);
}

static NT_ENTRY *findEntry(NT_ENTRY *entries, const size_t size,
                           const NT_STRING *key)
{
    NT_ENTRY *tombstone = NULL;
    size_t index = key->hash % size;
    for (;;)
    {
        NT_ENTRY *entry = &entries[index];

        if (entry->key == NULL)
        {
            if (entry->value == NULL)
                // empty entry
                return tombstone != NULL ? tombstone : entry;
            else if (tombstone == NULL)
                tombstone = entry;
        }
        else if (entry->key == key)
        {
            return entry;
        }

        index = (index + 1) % size;
    }
    return NULL;
}

static void adjustSize(NT_TABLE *table, const size_t size)
{
    NT_ENTRY *entries = (NT_ENTRY *)ntMalloc(size * sizeof(NT_ENTRY));
    for (size_t i = 0; i < size; ++i)
    {
        entries[i].key = NULL;
        entries[i].value = NULL;
    }

    table->count = 0;
    for (size_t i = 0; i < table->size; ++i)
    {
        const NT_ENTRY *entry = &table->pEntries[i];
        if (entry->key == NULL)
            continue;

        NT_ENTRY *dest = findEntry(entries, size, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    ntFree(table->pEntries);
    table->pEntries = entries;
    table->size = size;
}

void ntTableForAll(const NT_TABLE *table, tableCallback callback,
                   void *userdata)
{
    for (size_t i = 0; i < table->size; ++i)
    {
        const NT_ENTRY *entry = &table->pEntries[i];
        if (entry->key == NULL)
            continue;

        callback(entry->key, entry->value, userdata);
    }
}

bool ntTableSet(NT_TABLE *table, NT_STRING *key, void *value)
{
    if (table->count + 1 > table->size * TABLE_MAX_LOAD)
    {
        const size_t newSize = MAX(table->size * 3 / 2, 4);
        adjustSize(table, newSize);
    }

    NT_ENTRY *entry = findEntry(table->pEntries, table->size, key);
    const bool isNewKey = entry->key == NULL;

    // increment cout if is a new key and is not a tombstone
    if (isNewKey && entry->value == NULL)
        table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

void ntTableAddAll(const NT_TABLE *from, NT_TABLE *to)
{
    for (size_t i = 0; i < from->size; ++i)
    {
        const NT_ENTRY *entry = &from->pEntries[i];
        if (entry->key != NULL)
            ntTableSet(to, entry->key, entry->value);
    }
}

bool ntTableGet(const NT_TABLE *table, const NT_STRING *key, void **value)
{
    if (table->count == 0)
        return false;

    const NT_ENTRY *entry = findEntry(table->pEntries, table->size, key);
    if (entry->key == NULL)
        return false;

    *value = entry->value;
    return true;
}

bool ntTableDelete(NT_TABLE *table, NT_STRING *key, void **value)
{
    if (table->count == 0)
        return false;

    NT_ENTRY *entry = findEntry(table->pEntries, table->size, key);

    if (entry->key == NULL)
        return false;

    *value = entry->value;
    entry->key = NULL;
    entry->value = (void *)1;
    return true;
}

NT_STRING *ntTableFindString(const NT_TABLE *table, const char_t *chars,
                             size_t length, uint32_t hash)
{
    if (table->count == 0)
        return NULL;

    size_t index = hash % table->size;
    for (;;)
    {
        const NT_ENTRY *entry = &table->pEntries[index];
        if (entry->key == NULL)
        {
            // stop if find empty non-tombstone entry.
            if (entry->value == NULL)
                return NULL;
        }
        else if (entry->key->length == length && entry->key->hash == hash &&
                 memcmp(entry->key->chars, chars, length) == 0)
            return entry->key; // found it

        index = (index + 1) % table->size;
    }
    return NULL;
}
