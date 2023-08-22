#ifndef NIL_CONTEXT_H
#define NIL_CONTEXT_H

#include <netuno/common.h>

NT_HANDLE(NIL_CONTEXT)
NT_HANDLE(NT_STRING)

NIL_CONTEXT *nilCreateContext(void);
void nilDestroyContext(NIL_CONTEXT *c);
NT_STRING *nilGetPrefixedId(NIL_CONTEXT *context, const char_t *prefix);

#endif
