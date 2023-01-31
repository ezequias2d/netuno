#ifndef NT_DEBUG_H
#define NT_DEBUG_H

#include <netuno/assembly.h>
#include <netuno/module.h>

void ntDisassembleModule(const NT_ASSEMBLY *assembly, const NT_MODULE *module, const char *name);
size_t ntDisassembleInstruction(const NT_ASSEMBLY *assembly, const NT_MODULE *module,
                                const size_t offset);

#endif
