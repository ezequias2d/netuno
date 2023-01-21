#ifndef NETUNO_CODEGEN_H
#define NETUNO_CODEGEN_H

#include "parser.h"
#include "vstack.h"
#include <netuno/array.h>
#include <netuno/str.h>
#include <netuno/symbol.h>
#include <netuno/vm.h>

typedef struct
{
    NT_CHUNK *chunk;
    NT_ASSEMBLY *assembly;
    NT_SYMBOL_TABLE *scope;
    NT_VSTACK *stack;
    bool had_error;
} NT_CODEGEN;

NT_CODEGEN *ntCreateCodegen(NT_ASSEMBLY *assembly, NT_CHUNK *chunk);
void ntFreeCodegen(NT_CODEGEN *codegen);
bool ntGen(NT_CODEGEN *codegen, const NT_NODE **block, size_t count, const char_t *entryPointName,
           const NT_DELEGATE **entryPoint);

#endif
