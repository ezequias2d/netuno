#ifndef NT_OPCODE_H
#define NT_OPCODE_H

typedef enum
{
#define bytecode(a) BC_##a,
#include "opcode.inc"
#undef bytecode
    BC_LAST,
} NT_OPCODE;

#endif
