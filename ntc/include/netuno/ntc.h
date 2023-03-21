
#ifndef NT_COMPILER_H
#define NT_COMPILER_H

#include <netuno/assembly.h>
#include <netuno/common.h>

typedef struct _NT_FILE
{
    const char_t *code;
    const char_t *source;
    const char_t *filename;
} NT_FILE;

NT_ASSEMBLY *ntCompile(NT_ASSEMBLY *assembly, size_t fileCount, const NT_FILE *files);
#endif
