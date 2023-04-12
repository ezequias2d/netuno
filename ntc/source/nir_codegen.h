#ifndef NT_NIR_CODEGEN
#define NT_NIR_CODEGEN

#include "netuno/nir/basic_block.h"
#include "parser.h"
#include "report.h"
#include "scope.h"
#include <netuno/array.h>
#include <netuno/nir/context.h>
#include <netuno/str.h>

typedef struct
{
    NT_REPORT report;

    NIR_CONTEXT *context;
    NIR_FUNCTION *function;
    NIR_BASIC_BLOCK *block;

    NT_TYPE *tmodule;
    NIR_MODULE *module;
    NT_SCOPE *scope;
    NT_SCOPE *functionScope;
    bool public;
} NT_CODEGEN;

NIR_MODULE *ntNirGen(NIR_CONTEXT *context, const NT_NODE *moduleNode);

#endif
