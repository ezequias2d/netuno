#ifndef NT_CODEGEN_H
#define NT_CODEGEN_H

#include "parser.h"
#include "report.h"
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
    NT_REPORT report;

    NT_CODEGEN *codegen;
    NT_MODULE *module;
    NT_SYMBOL_TABLE *scope;
    NT_SYMBOL_TABLE *functionScope;
    NT_VSTACK *stack;
    bool public;
} NT_MODGEN;

NT_CODEGEN *ntCreateCodegen(NT_ASSEMBLY *assembly);
void ntFreeCodegen(NT_CODEGEN *codegen);
bool ntGen(NT_CODEGEN *codegen, size_t count, const NT_NODE **moduleNodes);

#endif
