#ifndef NT_RESOLVER_H
#define NT_RESOLVER_H

#include "netuno/symbol.h"
#include "parser.h"
#include "report.h"
#include <netuno/array.h>
#include <netuno/module.h>

const NT_TYPE *ntEvalBlockReturnType(NT_REPORT *report, NT_SYMBOL_TABLE *blockTable, NT_NODE *node);
const NT_TYPE *ntEvalExprType(NT_REPORT *report, NT_SYMBOL_TABLE *table, NT_NODE *node);
bool ntResolve(NT_ASSEMBLY *assembly, NT_SYMBOL_TABLE *globalTable, size_t moduleNodeCount,
               NT_NODE **moduleNodes);

#endif
