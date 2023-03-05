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
#ifndef NT_REPORT_H
#define NT_REPORT_H

#include <netuno/common.h>
#include <stdarg.h>

typedef struct _NT_NODE NT_NODE;
typedef struct _NT_TOKEN NT_TOKEN;
typedef struct _NT_REPORT
{
    bool had_error;
} NT_REPORT;

void ntVErrorAtToken(NT_TOKEN token, const char *message, va_list args);
void ntErrorAtToken(NT_TOKEN token, const char *message, ...);
void ntVWarningAtToken(NT_TOKEN token, const char *message, va_list args);
void ntWarningAtToken(NT_TOKEN token, const char *message, ...);
void ntVErrorAtNode(NT_REPORT *report, const NT_NODE *node, const char *message, va_list args);
void ntVWarningAtNode(const NT_NODE *node, const char *message, va_list args);
void ntErrorAtNode(NT_REPORT *report, const NT_NODE *node, const char *message, ...);
void ntWarningAtNode(const NT_NODE *node, const char *message, ...);

#endif
