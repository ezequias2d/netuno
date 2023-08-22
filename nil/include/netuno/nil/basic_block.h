#ifndef NIL_BASIC_BLOCK_H
#define NIL_BASIC_BLOCK_H

#include <netuno/common.h>

NT_HANDLE(NT_STRING)
NT_HANDLE(NIL_DEBUG_LOC)
NT_HANDLE(NIL_VALUE)
NT_HANDLE(NIL_FUNCTION)
NT_HANDLE(NIL_CONTEXT)
NT_HANDLE(NIL_BASIC_BLOCK)

NIL_BASIC_BLOCK *nilCreateBasicBlock(NIL_CONTEXT *context, const char_t *name);

const NT_STRING *nilGetBlockValueName(const NIL_BASIC_BLOCK *basicBlock);

NIL_CONTEXT *nilGetBlockContext(NIL_BASIC_BLOCK *block);
NIL_FUNCTION *nilGetBlockParent(NIL_BASIC_BLOCK *block);
void nilInsertBlockInto(NIL_BASIC_BLOCK *block, NIL_FUNCTION *function);

size_t nilGetBlockCount(NIL_BASIC_BLOCK *block);
NIL_VALUE *nilGetBlockValueByIndex(NIL_BASIC_BLOCK *block, size_t index);
NIL_VALUE *nilGetBlockTerminator(NIL_BASIC_BLOCK *block);

NIL_BASIC_BLOCK *nilGetPredecessor(NIL_BASIC_BLOCK *block, size_t index);
size_t nilGetPredecessorCount(NIL_BASIC_BLOCK *block);
NIL_BASIC_BLOCK *nilGetSinglePredecessor(NIL_BASIC_BLOCK *block);
NIL_BASIC_BLOCK *nilGetUniquePredecessor(NIL_BASIC_BLOCK *block);

void nilPrintBlock(NIL_BASIC_BLOCK *block);
#endif
