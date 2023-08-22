#ifndef NT_PBASIC_BLOCK_H
#define NT_PBASIC_BLOCK_H

#include "plist.h"
#include <netuno/nil/basic_block.h>

struct _NIL_BASIC_BLOCK
{
    NT_STRING *name;
    NIL_CONTEXT *context;
    NIL_FUNCTION *parent;
    NIL_VALUE *terminator;

    LIST(NIL_VALUE *, insts, instList);
    LIST(NIL_BASIC_BLOCK *, predecessors, predecessorList);
};

void insertInst(NIL_BASIC_BLOCK *block, NIL_VALUE *inst);

#endif
