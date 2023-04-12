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
#include "scope.h"
#include "netuno/string.h"
#include <assert.h>
#include <netuno/memory.h>
#include <string.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

NT_SCOPE *ntCreateSymbolTable(const NT_SCOPE *parent, NT_SCOPE_TYPE type, void *data)
{
    NT_SCOPE *symbolTable = (NT_SCOPE *)ntMalloc(sizeof(NT_SCOPE));
    ntInitSymbolTable(symbolTable, parent, type, data);
    return symbolTable;
}

void ntInitSymbolTable(NT_SCOPE *symbolTable, const NT_SCOPE *parent, NT_SCOPE_TYPE type,
                       void *data)
{
    symbolTable->parent = parent;
    symbolTable->data = data;
    symbolTable->count = 0;
    symbolTable->type = type;
    symbolTable->symbols = NULL;
    symbolTable->count = 0;
    symbolTable->max = 0;
    symbolTable->scopeReturnType = NULL;
    symbolTable->endLoop = NULL;
    symbolTable->loop = NULL;
}

void ntDeinitSymbolTable(NT_SCOPE *symbolTable)
{
    assert(symbolTable);

    ntFree(symbolTable->symbols);
}

void ntFreeSymbolTable(NT_SCOPE *symbolTable)
{
    if (symbolTable)
    {
        ntDeinitSymbolTable(symbolTable);
        ntFree(symbolTable);
    }
}

bool ntLookupSymbolCurrent(const NT_SCOPE *symbolTable, const NT_STRING *symbolName,
                           NT_SYMBOL *symbolEntry)
{
    for (size_t i = 0; i < symbolTable->count; i++)
    {
        const NT_SYMBOL current = symbolTable->symbols[i];

        if (ntStringEquals(current.symbol_name, symbolName))
        {
            if (symbolEntry)
                *symbolEntry = current;
            return true;
        }
    }
    return false;
}

bool ntLookupSymbolCurrent2(const NT_SCOPE *symbolTable, const char_t *symbolName,
                            const size_t symbolNameLen, NT_SYMBOL *symbolEntry)
{
    for (size_t i = 0; i < symbolTable->count; i++)
    {
        const NT_SYMBOL current = symbolTable->symbols[i];

        if (ntStringEquals2(current.symbol_name, symbolName, symbolNameLen))
        {
            if (symbolEntry)
                *symbolEntry = current;
            return true;
        }
    }
    return false;
}

bool ntLookupSymbol(const NT_SCOPE *symbolTable, const NT_STRING *symbolName,
                    NT_SCOPE **topSymbolTable, NT_SYMBOL *symbolEntry)
{
    if (ntLookupSymbolCurrent(symbolTable, symbolName, symbolEntry))
    {
        if (topSymbolTable)
            *topSymbolTable = (NT_SCOPE *)symbolTable;
        return true;
    }

    if (symbolTable->parent != NULL)
        return ntLookupSymbol(symbolTable->parent, symbolName, topSymbolTable, symbolEntry);
    return false;
}
bool ntLookupSymbol2(const NT_SCOPE *symbolTable, const char_t *symbolName,
                     const size_t symbolNameLen, NT_SCOPE **topSymbolTable, NT_SYMBOL *symbolEntry)
{
    if (ntLookupSymbolCurrent2(symbolTable, symbolName, symbolNameLen, symbolEntry))
    {
        if (topSymbolTable)
            *topSymbolTable = (NT_SCOPE *)symbolTable;
        return true;
    }

    if (symbolTable->parent != NULL)
        return ntLookupSymbol2(symbolTable->parent, symbolName, symbolNameLen, topSymbolTable,
                               symbolEntry);
    return false;
}

bool ntInsertSymbol(NT_SCOPE *symbolTable, const NT_SYMBOL *symbolEntry)
{
    NT_SYMBOL finded;
    if (ntLookupSymbolCurrent(symbolTable, symbolEntry->symbol_name, &finded))
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

    if (symbolTable->count + 1 > symbolTable->max)
    {
        const size_t newSize = MAX(symbolTable->max * 3 / 2, symbolTable->count + 1);
        symbolTable->symbols = ntRealloc(symbolTable->symbols, newSize * sizeof(NT_SYMBOL));
        symbolTable->max = newSize;
    }

    ntMemcpy(symbolTable->symbols + symbolTable->count, symbolEntry, sizeof(NT_SYMBOL));
    symbolTable->count++;

    return true;
}

bool ntUpdateSymbol(const NT_SCOPE *symbolTable, const NT_SYMBOL *symbolEntry)
{
    for (size_t i = 0; i < symbolTable->count; i++)
    {
        NT_SYMBOL *const current = &symbolTable->symbols[i];
        if (ntStringEquals(current->symbol_name, symbolEntry->symbol_name))
        {
            *current = *symbolEntry;

            NT_SYMBOL tmp;
            assert(ntLookupSymbolCurrent(symbolTable, symbolEntry->symbol_name, &tmp));
            return true;
        }
    }

    if (symbolTable->parent != NULL)
        return ntUpdateSymbol(symbolTable->parent, symbolEntry);
    return false;
}
