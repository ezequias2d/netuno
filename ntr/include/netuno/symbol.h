#ifndef NETUNO_SYMBOL_H
#define NETUNO_SYMBOL_H

#include "array.h"
#include "object.h"

typedef enum
{
    STT_NONE = 0,
    STT_BREAKABLE = 0x01,
    STT_FUNCTION = 0x02,
    STT_METHOD = 0x04,
} NT_SYMBOL_TABLE_TYPE;

typedef struct _NT_SYMBOL_TABLE NT_SYMBOL_TABLE;
struct _NT_SYMBOL_TABLE
{
    NT_SYMBOL_TABLE *parent;
    size_t data;
    uint32_t count;
    NT_SYMBOL_TABLE_TYPE type;
    NT_ARRAY *table;
};

typedef enum
{
    SYMBOL_TYPE_NONE,
    SYMBOL_TYPE_FUNCTION,
    SYMBOL_TYPE_SUBROUTINE,
    SYMBOL_TYPE_VARIABLE,
    SYMBOL_TYPE_CONSTANT,
    SYMBOL_TYPE_PARAM,
    SYMBOL_TYPE_TYPE,
} NT_SYMBOL_TYPE;

typedef struct _NT_SYMBOL_ENTRY
{
    const NT_STRING *symbol_name;
    NT_SYMBOL_TYPE type;
    size_t data;
    const NT_TYPE *exprType;
} NT_SYMBOL_ENTRY;

NT_SYMBOL_TABLE *ntCreateSymbolTable(NT_SYMBOL_TABLE *parent, NT_SYMBOL_TABLE_TYPE type,
                                     size_t data);
void ntFreeSymbolTable(NT_SYMBOL_TABLE *symbolTable);
bool ntLookupSymbol(NT_SYMBOL_TABLE *symbolTable, const char_t *symbolName,
                    const size_t symbolNameLen, NT_SYMBOL_ENTRY *symbolEntry);
bool ntInsertSymbol(NT_SYMBOL_TABLE *symbolTable, const NT_SYMBOL_ENTRY *symbolEntry);

#endif
