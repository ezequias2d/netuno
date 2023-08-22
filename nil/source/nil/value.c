#include "colors.h"
#include "netuno/memory.h"
#include "netuno/nil/basic_block.h"
#include "netuno/nil/instruction.h"
#include "netuno/nil/type.h"
#include "netuno/str.h"
#include "netuno/string.h"
#include "nil/pargument.h"
#include "nil/pconstant.h"
#include "nil/pinstruction.h"
#include "nil/ptype.h"
#include "pbasic_block.h"
#include "pfunction.h"
#include "pinstruction.h"
#include "pvalue.h"
#include <assert.h>
#include <inttypes.h>
#include <netuno/nil/value.h>
#include <stdio.h>

const NT_STRING *nilGetValueName(NIL_VALUE *value)
{
    assert(value);
    return value->name;
}

const NIL_DEBUG_LOC *nilGetValueDebugLoc(NIL_VALUE *value)
{
    assert(value);
    return value->dbgLoc;
}

bool nilIsValueType(NIL_VALUE *value, NIL_VALUE_TYPE type)
{
    assert(value);
    return value->valueType == type;
}

NIL_TYPE *nilGetType(NIL_VALUE *value)
{
    assert(value);
    return value->type;
}

static void printInst(NIL_INSTRUCTION *inst)
{
    if (inst->name)
    {
        nilPrintValueName((NIL_VALUE *)inst);
        printf(" = ");
    }

    NIL_OPCODE opcode = inst->opcode;
    printf(BLU "%s" reset, nilGetOpcodeName(opcode));

    if (nilIsBinaryOp(opcode))
    {
        NIL_BINARY_OPERATOR *bin = (NIL_BINARY_OPERATOR *)inst;

        printf(" ");
        nilPrintValueName(bin->source1);
        printf(", ");
        nilPrintValueName(bin->source2);
    }
    else if (nilIsUnaryOp(opcode))
    {
        NIL_UNARY_OPERATOR *un = (NIL_UNARY_OPERATOR *)inst;
        printf(" ");

        nilPrintType(un->type);
        printf(" ");

        nilPrintValueName(un->source);
    }
    else if (opcode == NIL_TERM_BR)
    {
        NIL_BRANCH_INST *br = (NIL_BRANCH_INST *)inst;

        if (br->condition)
        {
            printf(" ");
            nilPrintType(br->condition->type);
            printf(" ");
            nilPrintValueName(br->condition);
            printf(", " GRN "label" reset " " LABEL);
            ntPrintString(nilGetBlockValueName(br->ifTrue));
            printf(reset ", " GRN "label" reset " " LABEL);
            ntPrintString(nilGetBlockValueName(br->ifFalse));
            printf(reset);
        }
        else
        {
            printf(" " GRN "label" reset " " LABEL);
            ntPrintString(nilGetBlockValueName(br->ifTrue));
            printf(reset);
        }
    }
    else if (opcode == NIL_OTHER_CALL)
    {
        NIL_CALL_INST *call = (NIL_CALL_INST *)inst;

        printf(" ");
        nilPrintType(call->functionType->result);
        printf(" " YEL);

        ntPrintString(call->function->name);
        printf(reset "(");
        for (size_t i = 0; i < call->numArgs; ++i)
        {
            NIL_VALUE *arg = call->arguments[i];
            nilPrintType(arg->type);
            printf(" ");
            nilPrintValueName(arg);

            if (i + 1 < call->numArgs)
                printf(", ");
        }
        printf(")");
    }
    else if (opcode == NIL_OTHER_CMP)
    {
        NIL_CMP_INST *cmp = (NIL_CMP_INST *)inst;

        printf(" %s ", nilGetPredicateName(cmp->predicate));

        nilPrintType(cmp->source1->type);
        printf(" ");

        nilPrintValueName(cmp->source1);
        printf(", ");
        nilPrintValueName(cmp->source2);
    }
    else if (opcode == NIL_OTHER_PHI)
    {
        NIL_PHI_NODE *phi = (NIL_PHI_NODE *)inst;

        printf(" ");

        for (size_t i = 0; i < phi->list.count; ++i)
        {
            NIL_INCOMING_VALUE *in = phi->incomingValues[i];

            printf("[ ");

            nilPrintValueName(in->value);
            printf(", ");
            ntPrintString(in->block->name);

            printf("  ]");

            if (i + 1 < phi->list.count)
                printf(", ");
        }
    }
    else if (opcode == NIL_TERM_RET)
    {
        NIL_RETURN_INST *ret = (NIL_RETURN_INST *)inst;
        printf(" ");
        if (ret->retValue)
        {
            nilPrintType(ret->retValue->type);
            printf(" ");
            nilPrintValueName(ret->retValue);
        }
    }
    else if (opcode == NIL_OTHER_SELECT)
    {
        NIL_SELECT_INST *sel = (NIL_SELECT_INST *)inst;

        nilPrintValueName(sel->condition);
        printf(", ");
        nilPrintValueName(sel->trueValue);
        printf(", ");
        nilPrintValueName(sel->falseValue);
    }
    else if (opcode == NIL_OTHER_MEMORY_STORE)
    {
        NIL_STORE_INST *str = (NIL_STORE_INST *)inst;

        printf(" ");
        nilPrintValueName(str->source);
        printf(", [");
        nilPrintValueName(str->ptr);
        printf("]");
    }
    else
    {
        printf(" MISSING PRINTF");
    }
}

void nilPrintValue(NIL_VALUE *value)
{
    switch (value->valueType)
    {
    case NIL_VALUE_TYPE_ARGUMENT: {
        NIL_ARGUMENT *arg = (NIL_ARGUMENT *)value;
        printf("arg%zu", arg->argIndex);
        break;
    }
    case NIL_VALUE_TYPE_CONSTANT: {
        NIL_CONSTANT *c = (NIL_CONSTANT *)value;

        if (c->string)
        {
            assert(c->data);
            NIL_TYPE *charType = c->value.type;
            assert(nilIsIntegerNType(charType, sizeof(char_t) * 8));

            size_t length = c->numBytes / sizeof(char_t);
            char_t *unescape = ntUnescapeString((char_t *)c->data, &length);

            char *str = ntToCharFixed(unescape, length);
            ntFree(unescape);

            printf("\"%s\"", str);
            ntFree(str);
        }
        else if (nilIsIntegerType(c->value.type))
        {
            uint64_t tmp;
            switch (((NIL_INTEGER_TYPE *)c->value.type)->numBits / 8)
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
        else if (nilIsFloatType(c->value.type))
        {
            float tmp = *(float *)c->data;
            printf("#%f", tmp);
        }
        else if (nilIsDoubleType(c->value.type))
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
    case NIL_VALUE_TYPE_INSTRUCTION:
        printInst((NIL_INSTRUCTION *)value);
        break;
    }
}

void nilPrintValueName(NIL_VALUE *value)
{
    if (!value)
        printf("UNDEFINED");
    else if (value->name && value->valueType != NIL_VALUE_TYPE_CONSTANT)
    {
        printf(MAG "%%");
        ntPrintString(value->name);
        printf(reset);
    }
    else
        nilPrintValue(value);
}
