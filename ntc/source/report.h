#ifndef NT_REPORT_H
#define NT_REPORT_H

#include "parser.h"
#include <stdarg.h>

typedef struct
{
    bool had_error;
} NT_REPORT;

void ntVErrorAtNode(NT_REPORT *modgen, const NT_NODE *node, const char *message, va_list args);
void ntVWarningAtNode(const NT_NODE *node, const char *message, va_list args);
void ntErrorAtNode(NT_REPORT *modgen, const NT_NODE *node, const char *message, ...);
void ntWarningAtNode(const NT_NODE *node, const char *message, ...);

#endif
