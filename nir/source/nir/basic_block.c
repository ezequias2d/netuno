
#include "netuno/nir/basic_block.h"
#include "colors.h"
#include "netuno/str.h"
#include "nir/pinstruction.h"
#include "nir/plist.h"
#include "pbasic_block.h"
#include "pcontext.h"
#include "pfunction.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/nir/instruction.h>
#include <netuno/nir/value.h>
#include <netuno/string.h>
#include <stdio.h>
#include <string.h>

NIR_BASIC_BLOCK *nirCreateBasicBlock(NIR_CONTEXT *context, const char_t *name)
{
    assert(context);
    assert(name);

    NIR_BASIC_BLOCK *block = ntMalloc(sizeof(NIR_BASIC_BLOCK));

    const size_t nameLength = ntStrLen(name);
    const char_t entry[5] = U"entry";

    if (memcmp(name, entry, 5 * sizeof(char_t)) == 0)
        block->name = ntCopyString(name, nameLength);
    else
        block->name = nirGetPrefixedId(context, name);

    block->context = context;
    block->parent = NULL;
    block->terminator = NULL;

    listInit(&block->instList);
    listInit(&block->predecessorList);

    return block;
}

void insertInst(NIR_BASIC_BLOCK *block, NIR_VALUE *inst)
{
    assert(block);
    assert(inst);
    assert(block->terminator == NULL);
    assert(nirIsValueType(inst, NIR_VALUE_TYPE_INSTRUCTION));

    listAdd(&block->instList, inst);
    ((NIR_INSTRUCTION *)inst)->parent = block;

    if (nirIsTermiantor(nirGetOpcode(inst)))
        block->terminator = inst;
}

const NT_STRING *nirGetBlockValueName(const NIR_BASIC_BLOCK *block)
{
    assert(block);
    return block->name;
}

NIR_CONTEXT *nirGetBlockContext(NIR_BASIC_BLOCK *block)
{
    assert(block);
    return block->context;
}

NIR_FUNCTION *nirGetBlockParent(NIR_BASIC_BLOCK *block)
{
    assert(block);
    return block->parent;
}

size_t nirGetBlockCount(NIR_BASIC_BLOCK *block)
{
    assert(block);
    return block->instList.count;
}

NIR_VALUE *nirGetBlockValueByIndex(NIR_BASIC_BLOCK *block, size_t index)
{
    assert(index < block->instList.count);
    return block->insts[index];
}

NIR_VALUE *nirGetBlockLastValue(NIR_BASIC_BLOCK *block)
{
    const size_t count = block->instList.count;
    if (count == 0)
        return NULL;
    return block->insts[count - 1];
}

NIR_VALUE *nirGetBlockTerminator(NIR_BASIC_BLOCK *block)
{
    assert(block);
    return block->terminator;
}

void nirInsertBlockInto(NIR_BASIC_BLOCK *block, NIR_FUNCTION *function)
{
    assert(block);
    assert(block->parent == NULL);
    block->parent = function;
    listAdd(&function->list, block);
}

size_t nirGetPredecessorCount(NIR_BASIC_BLOCK *block)
{
    assert(block);
    return block->predecessorList.count;
}

NIR_BASIC_BLOCK *nirGetPredecessor(NIR_BASIC_BLOCK *block, size_t index)
{
    assert(block);
    assert(index < block->predecessorList.count);
    return block->predecessors[index];
}

NIR_BASIC_BLOCK *nirGetSinglePredecessor(NIR_BASIC_BLOCK *block)
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

NIR_BASIC_BLOCK *nirGetUniquePredecessor(NIR_BASIC_BLOCK *block)
{
    assert(block);

    // no predecessors.
    if (block->predecessorList.count == 0)
        return NULL;

    // unique predecessor
    NIR_BASIC_BLOCK *const constant = block->predecessors[0];
    for (size_t i = 1; i < block->predecessorList.count; ++i)
    {
        NIR_BASIC_BLOCK *const current = block->predecessors[i];
        if (current != constant)
            return NULL; // at least one is different
    }
    return constant;
}

void nirPrintBlock(NIR_BASIC_BLOCK *block)
{
    printf(LABEL);
    ntPrintString(block->name);
    printf(reset ":\n");

    for (size_t i = 0; i < block->instList.count; ++i)
    {
        printf("  ");

        NIR_VALUE *instruction = block->insts[i];
        if (instruction)
            nirPrintValue(instruction);
        else
            printf("NULL %zu", (size_t)(block->insts + i));

        printf("\n");
    }
}
