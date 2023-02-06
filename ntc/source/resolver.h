#ifndef NT_RESOLVER_H
#define NT_RESOLVER_H

#include "parser.h"
#include <netuno/array.h>
#include <netuno/module.h>

bool ntResolve(NT_ASSEMBLY *assembly, NT_SYMBOL_TABLE *globalTable, size_t moduleNodeCount,
               NT_NODE **moduleNodes);

#endif
