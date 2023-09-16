#ifndef NT_NIL_CODEGEN
#define NT_NIL_CODEGEN

#include "netuno/nil/basic_block.h"
#include "parser.h"
#include "report.h"
#include "scope.h"
#include <netuno/array.h>
#include <netuno/nil/context.h>
#include <netuno/str.h>

typedef struct
{
    NT_REPORT report;

    NIL_CONTEXT *context;
    NIL_FUNCTION *function;
    NIL_BASIC_BLOCK *block;

    NT_TYPE *tmodule;
    NIL_MODULE *module;
    NT_SCOPE *global;
    NT_SCOPE *scope;
    NT_SCOPE *functionScope;
    bool public;
} NT_CODEGEN;

NIL_MODULE *ntNirGen(NIL_CONTEXT *context, const NT_NODE *moduleNode);

#endif
