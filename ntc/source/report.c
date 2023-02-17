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
#include "report.h"
#include <netuno/memory.h>
#include <netuno/str.h>
#include <stdio.h>

void ntVErrorAtNode(NT_REPORT *modgen, const NT_NODE *node, const char *message, va_list args)
{
    const NT_TOKEN token = node->token;

    printf("[line %d] Error", token.line);

    if (token.type == TK_EOF)
        printf(" at end");
    else if (token.type == TK_ERROR)
    {
    }
    else
    {
        char *str = ntToCharFixed(token.lexeme, token.lexemeLength);
        printf(" at '%s'", str);
        ntFree(str);
    }

    printf(": ");
    vprintf(message, args);
    printf("\n");
    modgen->had_error = true;
}

void ntVWarningAtNode(const NT_NODE *node, const char *message, va_list args)
{
    const NT_TOKEN token = node->token;

    printf("[line %d] Warming", token.line);

    if (token.type == TK_EOF)
        printf(" at end");
    else if (token.type == TK_ERROR)
    {
    }
    else
    {
        char *str = ntToCharFixed(token.lexeme, token.lexemeLength);
        printf(" at '%s'", str);
        ntFree(str);
    }

    printf(": ");
    vprintf(message, args);
    printf("\n");
}

void ntErrorAtNode(NT_REPORT *modgen, const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    ntVErrorAtNode(modgen, node, message, vl);
    va_end(vl);
}

void ntWarningAtNode(const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    ntVWarningAtNode(node, message, vl);
    va_end(vl);
}
