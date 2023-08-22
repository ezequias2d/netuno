
#include "netuno/nil/basic_block.h"
#include "colors.h"
#include "netuno/str.h"
#include "nil/pinstruction.h"
#include "nil/plist.h"
#include "pbasic_block.h"
#include "pcontext.h"
#include "pfunction.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/nil/instruction.h>
#include <netuno/nil/value.h>
#include <netuno/string.h>
#include <stdio.h>
#include <string.h>

NIL_BASIC_BLOCK *nilCreateBasicBlock(NIL_CONTEXT *context, const char_t *name)
{
    assert(context);
    assert(name);

    NIL_BASIC_BLOCK *block = ntMalloc(sizeof(NIL_BASIC_BLOCK));

    const size_t nameLength = ntStrLen(name);
    const char_t entry[5] = U"entry";

    if (memcmp(name, entry, 5 * sizeof(char_t)) == 0)
        block->name = ntCopyString(name, nameLength);
    else
        block->name = nilGetPrefixedId(context, name);

    block->context = context;
    block->parent = NULL;
    block->terminator = NULL;

    listInit(&block->instList);
    listInit(&block->predecessorList);

    return block;
}

void insertInst(NIL_BASIC_BLOCK *block, NIL_VALUE *inst)
{
    assert(block);
    assert(inst);
    // assert(block->terminator == NULL);
    if (block->terminator != NULL)
        return;

    assert(nilIsValueType(inst, NIL_VALUE_TYPE_INSTRUCTION));

    listAdd(&block->instList, inst);
    ((NIL_INSTRUCTION *)inst)->parent = block;

    if (nilIsTermiantor(nilGetOpcode(inst)))
        block->terminator = inst;
}

const NT_STRING *nilGetBlockValueName(const NIL_BASIC_BLOCK *block)
{
    assert(block);
    return block->name;
}

NIL_CONTEXT *nilGetBlockContext(NIL_BASIC_BLOCK *block)
{
    assert(block);
    return block->context;
}

NIL_FUNCTION *nilGetBlockParent(NIL_BASIC_BLOCK *block)
{
    assert(block);
    return block->parent;
}

size_t nilGetBlockCount(NIL_BASIC_BLOCK *block)
{
    assert(block);
    return block->instList.count;
}

NIL_VALUE *nilGetBlockValueByIndex(NIL_BASIC_BLOCK *block, size_t index)
{
    assert(index < block->instList.count);
    return block->insts[index];
}

NIL_VALUE *nilGetBlockLastValue(NIL_BASIC_BLOCK *block)
{
    const size_t count = block->instList.count;
    if (count == 0)
        return NULL;
    return block->insts[count - 1];
}

NIL_VALUE *nilGetBlockTerminator(NIL_BASIC_BLOCK *block)
{
    assert(block);
    return block->terminator;
}

void nilInsertBlockInto(NIL_BASIC_BLOCK *block, NIL_FUNCTION *function)
{
    assert(block);
    assert(block->parent == NULL);
    block->parent = function;
    listAdd(&function->list, block);
}

size_t nilGetPredecessorCount(NIL_BASIC_BLOCK *block)
{
    assert(block);
    return block->predecessorList.count;
}

NIL_BASIC_BLOCK *nilGetPredecessor(NIL_BASIC_BLOCK *block, size_t index)
{
    assert(block);
    assert(index < block->predecessorList.count);
    return block->predecessors[index];
}

NIL_BASIC_BLOCK *nilGetSinglePredecessor(NIL_BASIC_BLOCK *block)
{
    assert(block);

    // no predecessors.
    if (block->predecessorList.count == 0)
        return NULL;

    // single predecessor
    if (block->predecessorList.count == 1)
        return block->predecessors[0];

    // multiple predecessors
    return NULL;
}

NIL_BASIC_BLOCK *nilGetUniquePredecessor(NIL_BASIC_BLOCK *block)
{
    assert(block);

    // no predecessors.
    if (block->predecessorList.count == 0)
        return NULL;

    // unique predecessor
    NIL_BASIC_BLOCK *const constant = block->predecessors[0];
    for (size_t i = 1; i < block->predecessorList.count; ++i)
    {
        NIL_BASIC_BLOCK *const current = block->predecessors[i];
        if (current != constant)
            return NULL; // at least one is different
    }
    return constant;
}

void nilPrintBlock(NIL_BASIC_BLOCK *block)
{
    printf(LABEL);
    ntPrintString(block->name);
    printf(reset ":\n");

    for (size_t i = 0; i < block->instList.count; ++i)
    {
        printf("  ");

        NIL_VALUE *instruction = block->insts[i];
        if (instruction)
            nilPrintValue(instruction);
        else
            printf("NULL %zu", (size_t)(block->insts + i));

        printf("\n");
    }
}
