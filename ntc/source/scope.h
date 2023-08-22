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
#ifndef NT_SCOPE_H
#define NT_SCOPE_H

#include "netuno/nil/instruction.h"
#include <netuno/common.h>
#include <netuno/string.h>
typedef struct _NT_TYPE NT_TYPE;

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
    SYMBOL_TYPE_MODULE = 256,
} NT_SYMBOL_TYPE;

typedef struct _NT_SYMBOL
{
    NT_STRING *symbol_name;
    NT_STRING *target_label; // for branch
    NT_SYMBOL_TYPE type;
    NIL_VALUE *value;
    const NT_TYPE *exprType;
    bool weak;
} NT_SYMBOL;

typedef enum
{
    STT_NONE = 0,
    STT_BREAKABLE = 0x01,
    STT_FUNCTION = 0x02,
    STT_METHOD = 0x04,
    STT_TYPE = 0x08,
} NT_SCOPE_TYPE;

typedef struct _NT_SCOPE NT_SCOPE;
struct _NT_SCOPE
{
    const NT_SCOPE *parent;
    void *data; // frame top
    const NT_TYPE *scopeReturnType;
    NT_SCOPE_TYPE type;

    size_t count;
    size_t max;
    NT_SYMBOL *symbols;

    NIL_BASIC_BLOCK *endLoop;
    NIL_BASIC_BLOCK *loop;
};

NT_SCOPE *ntCreateSymbolTable(const NT_SCOPE *parent, NT_SCOPE_TYPE type, void *data);
void ntInitSymbolTable(NT_SCOPE *table, const NT_SCOPE *parent, NT_SCOPE_TYPE type, void *data);
void ntDeinitSymbolTable(NT_SCOPE *symbolTable);
void ntFreeSymbolTable(NT_SCOPE *symbolTable);
bool ntLookupSymbol(const NT_SCOPE *symbolTable, const NT_STRING *symbolName,
                    NT_SCOPE **topSymbolTable, NT_SYMBOL *symbolEntry);
bool ntLookupSymbol2(const NT_SCOPE *symbolTable, const char_t *symbolName,
                     const size_t symbolNameLen, NT_SCOPE **topSymbolTable, NT_SYMBOL *symbolEntry);
bool ntLookupSymbolCurrent(const NT_SCOPE *symbolTable, const NT_STRING *symbolName,
                           NT_SYMBOL *symbolEntry);
bool ntLookupSymbolCurrent2(const NT_SCOPE *symbolTable, const char_t *symbolName,
                            const size_t symbolNameLen, NT_SYMBOL *symbolEntry);
bool ntInsertSymbol(NT_SCOPE *symbolTable, const NT_SYMBOL *symbolEntry);
bool ntUpdateSymbol(const NT_SCOPE *symbolTable, const NT_SYMBOL *symbolEntry);

#endif
