#ifndef NETUNO_DEBUG_H
#define NETUNO_DEBUG_H

#include "nto.h"

void ntDisassembleChunk(const NT_CHUNK *chunk, const char *name);
size_t ntDisassembleInstruction(const NT_CHUNK *chunk, const size_t offset);

#endif
