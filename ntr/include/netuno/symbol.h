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
#ifndef NT_SYMBOL_H
#define NT_SYMBOL_H

#include <netuno/array.h>

typedef struct _NT_TYPE NT_TYPE;
typedef struct _NT_STRING NT_STRING;

typedef enum
{
    STT_NONE = 0,
    STT_BREAKABLE = 0x01,
    STT_FUNCTION = 0x02,
    STT_METHOD = 0x04,
    STT_TYPE = 0x08,
} NT_SYMBOL_TABLE_TYPE;

typedef struct _NT_SYMBOL_TABLE NT_SYMBOL_TABLE;
struct _NT_SYMBOL_TABLE
{
    NT_SYMBOL_TABLE *parent;
    void *data; // frame top
    uint32_t count;
    const NT_TYPE *scopeReturnType;
    NT_SYMBOL_TABLE_TYPE type;
    NT_ARRAY *table;

    const NT_STRING *breakLabel;
    const NT_STRING *loopLabel;
};

typedef enum
{
    SYMBOL_TYPE_NONE = 0,
    SYMBOL_TYPE_FUNCTION = 1,
    SYMBOL_TYPE_SUBROUTINE = 2,
    SYMBOL_TYPE_VARIABLE = 4,
    SYMBOL_TYPE_CONSTANT = 8,
    SYMBOL_TYPE_PARAM = 16,
    SYMBOL_TYPE_TYPE = 32,
    SYMBOL_TYPE_PUBLIC = 64,
    SYMBOL_TYPE_PRIVATE = 128,
    SYMBOL_TYPE_LABEL = 256,
    SYMBOL_TYPE_BRANCH = 512,
    SYMBOL_TYPE_MODULE = 1024,
} NT_SYMBOL_TYPE;

typedef struct _NT_SYMBOL_ENTRY
{
    const NT_STRING *symbol_name;
    const NT_STRING *target_label; // for branch
    NT_SYMBOL_TYPE type;
    void *data;
    size_t data2;
    const NT_TYPE *exprType;
    bool weak;
} NT_SYMBOL_ENTRY;

NT_SYMBOL_TABLE *ntCreateSymbolTable(NT_SYMBOL_TABLE *parent, NT_SYMBOL_TABLE_TYPE type,
                                     void *data);
void ntInitSymbolTable(NT_SYMBOL_TABLE *table, NT_SYMBOL_TABLE *parent, NT_SYMBOL_TABLE_TYPE type,
                       void *data);
void ntDeinitSymbolTable(NT_SYMBOL_TABLE *symbolTable);
void ntFreeSymbolTable(NT_SYMBOL_TABLE *symbolTable);
bool ntLookupSymbol(const NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                    const size_t symbolNameLen, NT_SYMBOL_TABLE **topSymbolTable,
                    NT_SYMBOL_ENTRY *symbolEntry);
bool ntLookupSymbolCurrent(const NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                           const size_t symbolNameLen, NT_SYMBOL_ENTRY *symbolEntry);
bool ntInsertSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry);
bool ntUpdateSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry);

#endif
