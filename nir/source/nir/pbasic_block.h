#ifndef NT_PBASIC_BLOCK_H
#define NT_PBASIC_BLOCK_H

#include "plist.h"
#include <netuno/nir/basic_block.h>

struct _NIR_BASIC_BLOCK
{
    NT_STRING *name;
    NIR_CONTEXT *context;
    NIR_FUNCTION *parent;
    NIR_VALUE *terminator;

    LIST(NIR_VALUE *, insts, instList);
    LIST(NIR_BASIC_BLOCK *, predecessors, predecessorList);
};

void insertInst(NIR_BASIC_BLOCK *block, NIR_VALUE *inst);

#endif
