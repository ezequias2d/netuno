#ifndef NIR_BASIC_BLOCK_H
#define NIR_BASIC_BLOCK_H

#include <netuno/common.h>

NT_HANDLE(NT_STRING)
NT_HANDLE(NIR_DEBUG_LOC)
NT_HANDLE(NIR_VALUE)
NT_HANDLE(NIR_FUNCTION)
NT_HANDLE(NIR_CONTEXT)
NT_HANDLE(NIR_BASIC_BLOCK)

NIR_BASIC_BLOCK *nirCreateBasicBlock(NIR_CONTEXT *context, const char_t *name);

const NT_STRING *nirGetBlockValueName(const NIR_BASIC_BLOCK *basicBlock);

NIR_CONTEXT *nirGetBlockContext(NIR_BASIC_BLOCK *block);
NIR_FUNCTION *nirGetBlockParent(NIR_BASIC_BLOCK *block);
void nirInsertBlockInto(NIR_BASIC_BLOCK *block, NIR_FUNCTION *function);

size_t nirGetBlockCount(NIR_BASIC_BLOCK *block);
NIR_VALUE *nirGetBlockValueByIndex(NIR_BASIC_BLOCK *block, size_t index);
NIR_VALUE *nirGetBlockTerminator(NIR_BASIC_BLOCK *block);

NIR_BASIC_BLOCK *nirGetPredecessor(NIR_BASIC_BLOCK *block, size_t index);
size_t nirGetPredecessorCount(NIR_BASIC_BLOCK *block);
NIR_BASIC_BLOCK *nirGetSinglePredecessor(NIR_BASIC_BLOCK *block);
NIR_BASIC_BLOCK *nirGetUniquePredecessor(NIR_BASIC_BLOCK *block);

void nirPrintBlock(NIR_BASIC_BLOCK *block);
#endif
