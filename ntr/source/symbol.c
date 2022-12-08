#include <assert.h>
#include <netuno/memory.h>
#include <netuno/string.h>
#include <netuno/symbol.h>

NT_SYMBOL_TABLE *ntCreateSymbolTable(NT_SYMBOL_TABLE *parent, NT_SYMBOL_TABLE_TYPE type,
                                     size_t data)
{
    NT_SYMBOL_TABLE *symbolTable = (NT_SYMBOL_TABLE *)ntMalloc(sizeof(NT_SYMBOL_TABLE));
    symbolTable->parent = parent;
    symbolTable->data = data;
    symbolTable->count = 0;
    symbolTable->type = type;
    symbolTable->table = ntCreateArray();
    return symbolTable;
}

void ntFreeSymbolTable(NT_SYMBOL_TABLE *symbolTable)
{
    if (symbolTable)
    {
        ntFreeArray(symbolTable->table);
        ntFree(symbolTable);
    }
}

bool ntLookupSymbol(NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                    const size_t symbolNameLen, NT_SYMBOL_ENTRY *symbolEntry)
{
    for (size_t i = 0; i < symbolTable->table->count; i += sizeof(NT_SYMBOL_ENTRY))
    {
        NT_SYMBOL_ENTRY current;
        assert(ntArrayGet(symbolTable->table, i, &current, sizeof(NT_SYMBOL_ENTRY)) ==
               sizeof(NT_SYMBOL_ENTRY));
        if (ntStrEqualsFixed(current.symbol_name->chars, current.symbol_name->length, symbolName,
                             symbolNameLen))
        {
            if (symbolEntry)
                *symbolEntry = current;
            return true;
        }
    }

    if (symbolTable->parent != NULL)
        return ntLookupSymbol(symbolTable->parent, symbolName, symbolNameLen, symbolEntry);
    return false;
}

bool ntInsertSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry)
{
    if (ntLookupSymbol(symbolTable, symbolEntry->symbol_name->chars,
                       symbolEntry->symbol_name->length, NULL))
        return false;

    ntArrayAdd(symbolTable->table, symbolEntry, sizeof(NT_SYMBOL_ENTRY));
    return true;
}
