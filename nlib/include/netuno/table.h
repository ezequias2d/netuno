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
#ifndef NT_TABLE_H
#define NT_TABLE_H

#include <netuno/common.h>

typedef struct _NT_STRING NT_STRING;
typedef struct _NT_ENTRY NT_ENTRY;
typedef struct _NT_TABLE NT_TABLE;

struct _NT_ENTRY
{
    NT_STRING *key;
    void *value;
};

struct _NT_TABLE
{
    size_t count;
    size_t size;
    NT_ENTRY *pEntries;
};

typedef void (*tableCallback)(NT_STRING *key, void *value, void *userdata);

NT_TABLE *ntCreateTable(void);
void ntInitTable(NT_TABLE *table);
void ntDeinitTable(NT_TABLE *table);
void ntFreeTable(NT_TABLE *table);
void ntTableForAll(const NT_TABLE *table, tableCallback callback, void *userdata);
bool ntTableSet(NT_TABLE *table, NT_STRING *key, void *value);
void ntTableAddAll(const NT_TABLE *from, NT_TABLE *to);
bool ntTableGet(const NT_TABLE *table, const NT_STRING *key, void **value);
bool ntTableDelete(NT_TABLE *table, NT_STRING *key, void **value);
NT_STRING *ntTableFindString(const NT_TABLE *table, const char_t *chars, size_t length,
                             uint32_t hash);

#endif