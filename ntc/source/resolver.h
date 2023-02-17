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
