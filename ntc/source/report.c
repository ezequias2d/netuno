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
#include "parser.h"
#include "scanner.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <stdio.h>

// regular
#define BLK "\x1B[0;30m"
#define RED "\x1B[0;31m"
#define GRN "\x1B[0;32m"
#define YEL "\x1B[0;33m"
#define BLU "\x1B[0;34m"
#define MAG "\x1B[0;35m"
#define CYN "\x1B[0;36m"
#define WHT "\x1B[0;37m"

// regular background
#define BLKB "\x1B[40m"
#define REDB "\x1B[41m"
#define GRNB "\x1B[42m"
#define YELB "\x1B[43m"
#define BLUB "\x1B[44m"
#define MAGB "\x1B[45m"
#define CYNB "\x1B[46m"
#define WHTB "\x1B[47m"

// Reset
#define reset "\x1B[0m"

static size_t get_column(const char_t *line, const char_t *token)
{
    if (line == NULL || token == NULL)
        return 0;
    return token - line;
}

static void print_line_indicator(const char_t *line, const char_t *token)
{
    assert(line);

    size_t length;
    for (length = 0; line[length] != U'\0' && line[length] != U'\n'; ++length)
        ;

    char *temp = ntToCharFixed(line, length);
    printf("%s", temp);
    ntFree(temp);

    const size_t delta = get_column(line, token);
    printf("\n%*c" GRN "^" reset, (int)delta, ' ');
    printf("\n");
}

static void vmessageAtToken(NT_TOKEN token, const char *message, va_list args, bool error)
{
    const char *atEnd = (token.type == TK_EOF) ? " at end" : "";
    char *lexeme = "";
    bool freeLexeme = false;
    if (token.type != TK_EOF)
    {
        lexeme = ntToCharFixed(token.lexeme, token.lexemeLength);
        freeLexeme = true;
    }

    const size_t column = get_column(token.pLine, token.lexeme);

    char *sourcename = ntToChar(token.sourceName);
    printf("%s:%d:%zu", sourcename, token.line + 1, column + 1);
    ntFree(sourcename);

    if (error)
        printf(RED " error%s: " reset, atEnd);
    else
        printf(MAG " warning%s: " reset, atEnd);

    vprintf(message, args);
    printf("\n");

    print_line_indicator(token.pLine, token.lexeme);

    if (freeLexeme)
        ntFree(lexeme);

    // assert(!error);
}

void ntVErrorAtToken(NT_TOKEN token, const char *message, va_list args)
{
    vmessageAtToken(token, message, args, true);
}

void ntErrorAtToken(NT_TOKEN token, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    ntVErrorAtToken(token, message, vl);
    va_end(vl);
}

void ntVWarningAtToken(NT_TOKEN token, const char *message, va_list args)
{
    vmessageAtToken(token, message, args, false);
}

void ntWarningAtToken(NT_TOKEN token, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    ntVWarningAtToken(token, message, vl);
    va_end(vl);
}

void ntVErrorAtNode(NT_REPORT *report, const NT_NODE *node, const char *message, va_list args)
{
    ntVErrorAtToken(node->token, message, args);
    report->had_error = true;
}

void ntVWarningAtNode(const NT_NODE *node, const char *message, va_list args)
{
    ntWarningAtToken(node->token, message, args);
}

void ntErrorAtNode(NT_REPORT *report, const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    ntVErrorAtNode(report, node, message, vl);
    va_end(vl);
}

void ntWarningAtNode(const NT_NODE *node, const char *message, ...)
{
    va_list vl;
    va_start(vl, message);
    ntVWarningAtNode(node, message, vl);
    va_end(vl);
}
