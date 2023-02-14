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
