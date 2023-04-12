#include "colors.h"
#include "netuno/nir/basic_block.h"
#include "netuno/nir/instruction.h"
#include "netuno/nir/type.h"
#include "netuno/string.h"
#include "nir/pargument.h"
#include "nir/pconstant.h"
#include "nir/pinstruction.h"
#include "nir/ptype.h"
#include "pbasic_block.h"
#include "pfunction.h"
#include "pinstruction.h"
#include "pvalue.h"
#include <assert.h>
#include <inttypes.h>
#include <netuno/nir/value.h>
#include <stdio.h>

const NT_STRING *nirGetValueName(NIR_VALUE *value)
{
    assert(value);
    return value->name;
}

const NIR_DEBUG_LOC *nirGetValueDebugLoc(NIR_VALUE *value)
{
    assert(value);
    return value->dbgLoc;
}

bool nirIsValueType(NIR_VALUE *value, NIR_VALUE_TYPE type)
{
    assert(value);
    return value->valueType == type;
}

NIR_TYPE *nirGetType(NIR_VALUE *value)
{
    assert(value);
    return value->type;
}

static void printInst(NIR_INSTRUCTION *inst)
{
    if (inst->name)
    {
        nirPrintValueName((NIR_VALUE *)inst);
        printf(" = ");
    }

    NIR_OPCODE opcode = inst->opcode;
    printf(BLU "%s" reset, nirGetOpcodeName(opcode));

    if (nirIsBinaryOp(opcode))
    {
        NIR_BINARY_OPERATOR *bin = (NIR_BINARY_OPERATOR *)inst;

        printf(" ");
        nirPrintValueName(bin->source1);
        printf(", ");
        nirPrintValueName(bin->source2);
    }
    else if (nirIsUnaryOp(opcode))
    {
        NIR_UNARY_OPERATOR *un = (NIR_UNARY_OPERATOR *)inst;
        printf(" ");

        nirPrintType(un->type);
        printf(" ");

        nirPrintValueName(un->source);
    }
    else if (opcode == NIR_TERM_BR)
    {
        NIR_BRANCH_INST *br = (NIR_BRANCH_INST *)inst;

        if (br->condition)
        {
            printf(" ");
            nirPrintType(br->condition->type);
            printf(" ");
            nirPrintValueName(br->condition);
            printf(", " GRN "label" reset " " LABEL);
            ntPrintString(nirGetBlockValueName(br->ifTrue));
            printf(reset ", " GRN "label" reset " " LABEL);
            ntPrintString(nirGetBlockValueName(br->ifFalse));
            printf(reset);
        }
        else
        {
            printf(" " GRN "label" reset " " LABEL);
            ntPrintString(nirGetBlockValueName(br->ifTrue));
            printf(reset);
        }
    }
    else if (opcode == NIR_OTHER_CALL)
    {
        NIR_CALL_INST *call = (NIR_CALL_INST *)inst;

        printf(" ");
        nirPrintType(call->functionType->result);
        printf(" " YEL);

        ntPrintString(call->function->name);
        printf(reset "(");
        for (size_t i = 0; i < call->numArgs; ++i)
        {
            NIR_VALUE *arg = call->arguments[i];
            nirPrintType(arg->type);
            printf(" ");
            nirPrintValueName(arg);

            if (i + 1 < call->numArgs)
                printf(", ");
        }
        printf(")");
    }
    else if (opcode == NIR_OTHER_CMP)
    {
        NIR_CMP_INST *cmp = (NIR_CMP_INST *)inst;

        printf(" %s ", nirGetPredicateName(cmp->predicate));

        nirPrintType(cmp->source1->type);
        printf(" ");

        nirPrintValueName(cmp->source1);
        printf(", ");
        nirPrintValueName(cmp->source2);
    }
    else if (opcode == NIR_OTHER_PHI)
    {
        NIR_PHI_NODE *phi = (NIR_PHI_NODE *)inst;

        printf(" ");

        for (size_t i = 0; i < phi->list.count; ++i)
        {
            NIR_INCOMING_VALUE *in = phi->incomingValues[i];

            printf("[ ");

            nirPrintValueName(in->value);
            printf(", ");
            ntPrintString(in->block->name);

            printf("  ]");

            if (i + 1 < phi->list.count)
                printf(", ");
        }
    }
    else if (opcode == NIR_TERM_RET)
    {
        NIR_RETURN_INST *ret = (NIR_RETURN_INST *)inst;
        printf(" ");
        if (ret->retValue)
        {
            nirPrintType(ret->retValue->type);
            printf(" ");
            nirPrintValueName(ret->retValue);
        }
    }
    else if (opcode == NIR_OTHER_SELECT)
    {
        NIR_SELECT_INST *sel = (NIR_SELECT_INST *)inst;

        nirPrintValueName(sel->condition);
        printf(", ");
        nirPrintValueName(sel->trueValue);
        printf(", ");
        nirPrintValueName(sel->falseValue);
    }
    else if (opcode == NIR_OTHER_MEMORY_STORE)
    {
        NIR_STORE_INST *str = (NIR_STORE_INST *)inst;

        printf(" ");
        nirPrintValueName(str->source);
        printf(", [");
        nirPrintValueName(str->ptr);
        printf("]");
    }
    else
    {
        printf(" MISSING PRINTF");
    }
}

void nirPrintValue(NIR_VALUE *value)
{
    switch (value->valueType)
    {
    case NIR_VALUE_TYPE_ARGUMENT: {
        NIR_ARGUMENT *arg = (NIR_ARGUMENT *)value;
        printf("arg%zu", arg->argIndex);
        break;
    }
    case NIR_VALUE_TYPE_CONSTANT: {
        NIR_CONSTANT *c = (NIR_CONSTANT *)value;

        if (nirIsIntegerType(c->value.type))
        {
            uint64_t tmp;
            switch (((NIR_INTEGER_TYPE *)c->value.type)->numBits / 8)
            {
            case 0:
                tmp = *(uint8_t *)c->data;
                printf("#%lu", tmp);
                break;
            case 4:
                tmp = *(uint32_t *)c->data;
                printf("#%lu", tmp);
                break;
            case 8:
                tmp = *(uint64_t *)c->data;
                printf("#%lu", tmp);
                break;
            default:
                goto undefined_constant;
                break;
            }
        }
        else if (nirIsFloatType(c->value.type))
        {
            float tmp = *(float *)c->data;
            printf("#%f", tmp);
        }
        else if (nirIsDoubleType(c->value.type))
        {
            double tmp = *(float *)c->data;
            printf("#%f", tmp);
        }
        else
        {
            goto undefined_constant;
        }
        return;
    undefined_constant:
        for (size_t i = 0; i < c->numBytes; i++)
            printf("%02X", c->data[i]);
        break;
    }
    case NIR_VALUE_TYPE_INSTRUCTION:
        printInst((NIR_INSTRUCTION *)value);
        break;
    }
}

void nirPrintValueName(NIR_VALUE *value)
{
    if (!value)
        printf("UNDEFINED");
    else if (value->name && value->valueType != NIR_VALUE_TYPE_CONSTANT)
    {
        printf(MAG "%%");
        ntPrintString(value->name);
        printf(reset);
    }
    else
        nirPrintValue(value);
}
