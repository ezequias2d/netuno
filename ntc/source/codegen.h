#ifndef NETUNO_CODEGEN_H
#define NETUNO_CODEGEN_H

#include "parser.h"
#include "vstack.h"
#include <netuno/array.h>
#include <netuno/assembly.h>
#include <netuno/str.h>
#include <netuno/symbol.h>
#include <netuno/vm.h>

typedef struct
{
    NT_ASSEMBLY *assembly;
    bool had_error;
} NT_CODEGEN;

typedef struct
{
    NT_CODEGEN *codegen;
    NT_MODULE *module;
    NT_SYMBOL_TABLE *scope;
    NT_SYMBOL_TABLE *functionScope;
    NT_VSTACK *stack;
    bool public;
    bool had_error;
} NT_MODGEN;

NT_CODEGEN *ntCreateCodegen(NT_ASSEMBLY *assembly);
void ntFreeCodegen(NT_CODEGEN *codegen);
bool ntGen(NT_CODEGEN *codegen, const NT_NODE **block, size_t count);

#endif
