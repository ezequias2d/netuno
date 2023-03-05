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
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/string.h>
#include <netuno/symbol.h>

NT_SYMBOL_TABLE *ntCreateSymbolTable(NT_SYMBOL_TABLE *parent, NT_SYMBOL_TABLE_TYPE type, void *data)
{
    NT_SYMBOL_TABLE *symbolTable = (NT_SYMBOL_TABLE *)ntMalloc(sizeof(NT_SYMBOL_TABLE));
    ntInitSymbolTable(symbolTable, parent, type, data);
    return symbolTable;
}

void ntInitSymbolTable(NT_SYMBOL_TABLE *symbolTable, NT_SYMBOL_TABLE *parent,
                       NT_SYMBOL_TABLE_TYPE type, void *data)
{
    symbolTable->parent = parent;
    symbolTable->data = data;
    symbolTable->count = 0;
    symbolTable->type = type;
    symbolTable->table = ntCreateArray();
    symbolTable->scopeReturnType = NULL;
    symbolTable->breaked = false;
    symbolTable->continued = false;
    symbolTable->breakLabel = NULL;
    symbolTable->loopLabel = NULL;
}

void ntDeinitSymbolTable(NT_SYMBOL_TABLE *symbolTable)
{
    assert(symbolTable);
    ntFreeArray(symbolTable->table);

    if (symbolTable->loopLabel)
        ntFreeObject((NT_OBJECT *)symbolTable->loopLabel);

    if (symbolTable->breakLabel)
        ntFreeObject((NT_OBJECT *)symbolTable->breakLabel);
}

void ntFreeSymbolTable(NT_SYMBOL_TABLE *symbolTable)
{
    if (symbolTable)
    {
        ntDeinitSymbolTable(symbolTable);
        ntFree(symbolTable);
    }
}

bool ntLookupSymbolCurrent(const NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                           const size_t symbolNameLen, NT_SYMBOL_ENTRY *symbolEntry)
{
    for (size_t i = 0; i < symbolTable->table->count; i += sizeof(NT_SYMBOL_ENTRY))
    {
        NT_SYMBOL_ENTRY current;
        const bool result = ntArrayGet(symbolTable->table, i, &current, sizeof(NT_SYMBOL_ENTRY)) ==
                            sizeof(NT_SYMBOL_ENTRY);
        assert(result);
        if (ntStrEqualsFixed(current.symbol_name->chars, current.symbol_name->length, symbolName,
                             symbolNameLen))
        {
            if (symbolEntry)
                *symbolEntry = current;
            return true;
        }
    }
    return false;
}

bool ntLookupSymbol(const NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                    const size_t symbolNameLen, NT_SYMBOL_TABLE **topSymbolTable,
                    NT_SYMBOL_ENTRY *symbolEntry)
{
    if (ntLookupSymbolCurrent(symbolTable, symbolName, symbolNameLen, symbolEntry))
    {
        if (topSymbolTable)
            *topSymbolTable = (NT_SYMBOL_TABLE *)symbolTable;
        return true;
    }

    if (symbolTable->parent != NULL)
        return ntLookupSymbol(symbolTable->parent, symbolName, symbolNameLen, topSymbolTable,
                              symbolEntry);
    return false;
}

bool ntInsertSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry)
{
    NT_SYMBOL_ENTRY finded;
    if (ntLookupSymbolCurrent(symbolTable, symbolEntry->symbol_name->chars,
                              symbolEntry->symbol_name->length, &finded))
    {
        // update symbol if current is weak and symbolEntry is not weak
        if (finded.weak && !symbolEntry->weak)
        {
            // update and turns in a no weak symbol
            ntUpdateSymbol(symbolTable, symbolEntry);
            return true;
        }

        return false;
    }
    ntArrayAdd(symbolTable->table, symbolEntry, sizeof(NT_SYMBOL_ENTRY));
    return true;
}

bool ntUpdateSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry)
{
    for (size_t i = 0; i < symbolTable->table->count; i += sizeof(NT_SYMBOL_ENTRY))
    {
        NT_SYMBOL_ENTRY current;
        const bool result = ntArrayGet(symbolTable->table, i, &current, sizeof(NT_SYMBOL_ENTRY)) ==
                            sizeof(NT_SYMBOL_ENTRY);
        assert(result);
        if (ntStrEqualsFixed(current.symbol_name->chars, current.symbol_name->length,
                             symbolEntry->symbol_name->chars, symbolEntry->symbol_name->length))
        {
            ntArraySet(symbolTable->table, i, symbolEntry, sizeof(NT_SYMBOL_ENTRY));

            NT_SYMBOL_ENTRY tmp;
            ntLookupSymbolCurrent(symbolTable, symbolEntry->symbol_name->chars,
                                  symbolEntry->symbol_name->length, &tmp);

            return true;
        }
    }

    if (symbolTable->parent != NULL)
        return ntUpdateSymbol(symbolTable->parent, symbolEntry);
    return false;
}
