
#ifndef NETUNO_COMPILER_H
#define NETUNO_COMPILER_H

#include <netuno/common.h>
#include <netuno/nto.h>

NT_CHUNK *ntCompile(NT_ASSEMBLY *assembly, const char_t *str, const char_t *entryPointName,
                    const NT_DELEGATE **entryPoint);
#endif
