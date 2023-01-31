
#ifndef NT_COMPILER_H
#define NT_COMPILER_H

#include <netuno/assembly.h>
#include <netuno/common.h>

NT_ASSEMBLY *ntCompile(NT_ASSEMBLY *assembly, const char_t *str, const char_t *sourceName);
#endif
