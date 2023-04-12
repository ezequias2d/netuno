
#ifndef NT_COMPILER_H
#define NT_COMPILER_H

#include <netuno/nir/module.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uchar.h>

typedef char32_t char_t;

typedef struct _NT_FILE
{
    const char_t *code;
    const char_t *source;
    const char_t *filename;
} NT_FILE;

bool ntCompile(size_t fileCount, const NT_FILE *files, NIR_MODULE **modules);
#endif
