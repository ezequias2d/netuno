#ifndef NIR_CONTEXT_H
#define NIR_CONTEXT_H

#include <netuno/common.h>

NT_HANDLE(NIR_CONTEXT)
NT_HANDLE(NT_STRING)

NIR_CONTEXT *nirCreateContext(void);
void nirDestroyContext(NIR_CONTEXT *c);
NT_STRING *nirGetPrefixedId(NIR_CONTEXT *context, const char_t *prefix);

#endif
