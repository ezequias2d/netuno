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
}

void ntDeinitSymbolTable(NT_SYMBOL_TABLE *symbolTable)
{
    assert(symbolTable);
    ntFreeArray(symbolTable->table);
}

void ntFreeSymbolTable(NT_SYMBOL_TABLE *symbolTable)
{
    if (symbolTable)
    {
        ntDeinitSymbolTable(symbolTable);
        ntFree(symbolTable);
    }
}

bool ntLookupSymbolCurrent(NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
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

bool ntLookupSymbol(NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                    const size_t symbolNameLen, NT_SYMBOL_TABLE **topSymbolTable,
                    NT_SYMBOL_ENTRY *symbolEntry)
{
    if (ntLookupSymbolCurrent(symbolTable, symbolName, symbolNameLen, symbolEntry))
    {
        if (topSymbolTable)
            *topSymbolTable = symbolTable;
        return true;
    }

    if (symbolTable->parent != NULL)
        return ntLookupSymbol(symbolTable->parent, symbolName, symbolNameLen, topSymbolTable,
                              symbolEntry);
    return false;
}

bool ntInsertSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry)
{
    if (ntLookupSymbolCurrent(symbolTable, symbolEntry->symbol_name->chars,
                              symbolEntry->symbol_name->length, NULL))
        return false;

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
