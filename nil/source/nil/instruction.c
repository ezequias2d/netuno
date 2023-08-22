
#include "netuno/nil/instruction.h"
#include "netuno/memory.h"
#include "netuno/nil/basic_block.h"
#include "netuno/nil/constant.h"
#include "netuno/nil/context.h"
#include "netuno/nil/type.h"
#include "netuno/nil/value.h"
#include "netuno/string.h"
#include "nil/pbasic_block.h"
#include "nil/pcontext.h"
#include "nil/plist.h"
#include "pinstruction.h"
#include "ptype.h"
#include <assert.h>

const char *nilGetOpcodeName(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case (NIL_TERM_RET):
        return "ret";
    case (NIL_TERM_BR):
        return "br";

    // unary operators
    case (NIL_UNARY_FNEG):
        return "fneg";
    case (NIL_UNARY_MEMORY_ALLOCA):
        return "alloca";
    case (NIL_UNARY_MEMORY_LOAD):
        return "load";
    case (NIL_UNARY_CAST_TRUNC):
        return "trunc";
    case (NIL_UNARY_CAST_ZEXT):
        return "zext";
    case (NIL_UNARY_CAST_SEXT):
        return "sext";
    case (NIL_UNARY_CAST_FP_TO_UI):
        return "fp_to_ui";
    case (NIL_UNARY_CAST_FP_TO_SI):
        return "fp_to_si";
    case (NIL_UNARY_CAST_UI_TO_FP):
        return "ui_to_fp";
    case (NIL_UNARY_CAST_SI_TO_FP):
        return "si_to_fp";
    case (NIL_UNARY_CAST_FP_TRUNC):
        return "fp_trunc";
    case (NIL_UNARY_CAST_PTR_TO_INT):
        return "ptr_to_int";
    case (NIL_UNARY_CAST_INT_TO_PTR):
        return "int_to_ptr";
    case (NIL_UNARY_CAST_BIT_CAST):
        return "bit_cast";

    // binary operators
    case (NIL_BINARY_OP_ADD):
        return "add";
    case (NIL_BINARY_OP_FADD):
        return "fadd";
    case (NIL_BINARY_OP_SUB):
        return "sub";
    case (NIL_BINARY_OP_FSUB):
        return "fsub";
    case (NIL_BINARY_OP_MUL):
        return "mul";
    case (NIL_BINARY_OP_FMUL):
        return "fmul";
    case (NIL_BINARY_OP_UDIV):
        return "udiv";
    case (NIL_BINARY_OP_SDIV):
        return "sdiv";
    case (NIL_BINARY_OP_FDIV):
        return "fdiv";
    case (NIL_BINARY_OP_UREM):
        return "urem";
    case (NIL_BINARY_OP_SREM):
        return "srem";
    case (NIL_BINARY_OP_FREM):
        return "frem";

    // logical operators
    case (NIL_BINARY_OP_SHL):
        return "shl";
    case (NIL_BINARY_OP_SHR):
        return "shr";
    case (NIL_BINARY_OP_ASR):
        return "asr";
    case (NIL_BINARY_OP_AND):
        return "and";
    case (NIL_BINARY_OP_OR):
        return "or";
    case (NIL_BINARY_OP_XOR):
        return "xor";

    // others
    case (NIL_GET_ELEMENT_PTR):
        return "get_element_ptr";
    case (NIL_EXTRACT_VALUE):
        return "extract_value";
    case (NIL_OTHER_CMP):
        return "cmp";
    case (NIL_OTHER_PHI):
        return "phi";
    case (NIL_OTHER_CALL):
        return "call";
    case (NIL_OTHER_SELECT):
        return "select";
    case NIL_OTHER_MEMORY_STORE:
        return "store";

    case (NIL_OTHER_INSERT_VALUE):
        return "insert_value";

    default:
        assert(false);
        return NULL;
    }
}

bool nilIsTermiantor(NIL_OPCODE opcode)
{
    return opcode >= NIL_TERM_OPS_BEGIN && opcode < NIL_TERM_OPS_END;
}

bool nilIsUnaryOp(NIL_OPCODE opcode)
{
    return opcode >= NIL_UNARY_OPS_BEGIN && opcode < NIL_UNARY_OPS_END;
}

bool nilIsBinaryOp(NIL_OPCODE opcode)
{
    return opcode >= NIL_BINARY_OPS_BEGIN && opcode < NIL_BINARY_OPS_END;
}

bool nilIsIntDivRem(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_UDIV:
    case NIL_BINARY_OP_SDIV:
    case NIL_BINARY_OP_FDIV:
    case NIL_BINARY_OP_UREM:
    case NIL_BINARY_OP_SREM:
    case NIL_BINARY_OP_FREM:
        return true;
    default:
        return false;
    }
}

bool nilIsShift(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_SHL:
    case NIL_BINARY_OP_SHR:
    case NIL_BINARY_OP_ASR:
        return true;
    default:
        return false;
    }
}

bool nilIsLogicalShift(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_SHL:
    case NIL_BINARY_OP_SHR:
        return true;
    default:
        return false;
    }
}

bool nilIsArithmeticShift(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_SHL:
    case NIL_BINARY_OP_ASR:
        return true;
    default:
        return false;
    }
}

bool nilIsBitwiseLogicOp(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_AND:
    case NIL_BINARY_OP_OR:
    case NIL_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

bool nilIsCast(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_UNARY_CAST_TRUNC:
    case NIL_UNARY_CAST_ZEXT:
    case NIL_UNARY_CAST_SEXT:
    case NIL_UNARY_CAST_FP_TO_UI:
    case NIL_UNARY_CAST_FP_TO_SI:
    case NIL_UNARY_CAST_UI_TO_FP:
    case NIL_UNARY_CAST_SI_TO_FP:
    case NIL_UNARY_CAST_FP_TRUNC:
    case NIL_UNARY_CAST_PTR_TO_INT:
    case NIL_UNARY_CAST_INT_TO_PTR:
    case NIL_UNARY_CAST_BIT_CAST:
        return true;
    default:
        return false;
    }
}

bool nilIsAssociative(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_ADD:
    case NIL_BINARY_OP_MUL:
    case NIL_BINARY_OP_AND:
    case NIL_BINARY_OP_OR:
    case NIL_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

bool nilIsCommutative(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_ADD:
    case NIL_BINARY_OP_FADD:
    case NIL_BINARY_OP_MUL:
    case NIL_BINARY_OP_FMUL:
    case NIL_BINARY_OP_AND:
    case NIL_BINARY_OP_OR:
    case NIL_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

bool nilIsIdempotent(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_AND:
    case NIL_BINARY_OP_OR:
        return true;
    default:
        return false;
    }
}

bool nilIsNilpotent(NIL_OPCODE opcode)
{
    switch (opcode)
    {
    case NIL_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

NIL_OPCODE nilGetOpcode(NIL_VALUE *instruction)
{
    TO_INST(instruction);
    return inst->opcode;
}

/**********/
/* Binary */
/**********/

NIL_VALUE *nilCreateBinary(NIL_OPCODE op, NIL_VALUE *source1,
                           NIL_VALUE *source2, const char_t *name,
                           NIL_BASIC_BLOCK *block)
{
    assert(nilIsBinaryOp(op));
    // assert(!source1 || !source2 ||
    //        nilGetType(source1) == nilGetType(source2) &&
    //            "operand1 and operand2 must has same type");

    NIL_CONTEXT *const context = nilGetBlockContext(block);

    NIL_BINARY_OPERATOR *const bin = ntMalloc(sizeof(NIL_BINARY_OPERATOR));

    // value
    bin->name = nilGetPrefixedId(context, name);
    bin->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    bin->dbgLoc = NULL;
    bin->type = source1 ? nilGetType(source1) : NULL;
    bin->parent = NULL;

    // instruction
    bin->opcode = op;

    // binary operator
    bin->source1 = source1;
    bin->source2 = source2;

    insertInst(block, (NIL_VALUE *)bin);

    return (NIL_VALUE *)bin;
}

NIL_VALUE *nilCreateNeg(NIL_VALUE *source, const char_t *name,
                        NIL_BASIC_BLOCK *block)
{
    NIL_VALUE *const zero = nilGetInt(nilGetType(source), 0, false);
    return nilCreateBinary(NIL_BINARY_OP_SUB, zero, source, name, block);
}

NIL_VALUE *nilCreateNot(NIL_VALUE *source, const char_t *name,
                        NIL_BASIC_BLOCK *block)
{
    NIL_VALUE *const ones = nilGetIntAllOnes(nilGetType(source));
    return nilCreateBinary(NIL_BINARY_OP_XOR, ones, source, name, block);
}

/**********/
/* Branch */
/**********/

/**
 * @brief Construct a uncoditional branch instruction, given destination
 * BasicBlock. Also automatically insert this instruction to the end of the
 * BasicBlock specified.
 *
 * @param destBasicBlock Destination BasicBlock.
 * @param basicBlock Basic block to insert.
 * @return NIL_VALUE* Value that represents the instruction.
 */
NIL_VALUE *nilCreateBranch1(NIL_BASIC_BLOCK *destBasicBlock,
                            NIL_BASIC_BLOCK *block)
{
    return nilCreateBranch2(destBasicBlock, NULL, NULL, block);
}

/**
 * @brief Construct a coditional branch instruction, given destination true
 * BasicBlock, false BasicBlock and condition. Also automatically insert this
 * instruction to the end of the BasicBlock specified.
 *
 * @param ifTrueBasicBlock Destination BasicBlock when codition are true.
 * @param ifFalseBasicBlock Destination BasicBlock when codition are false.
 * @param cond Condition value, must be i1.
 * @param basicBlock Basic block to insert.
 * @return NIL_VALUE* Value that represents the instruction.
 */
NIL_VALUE *nilCreateBranch2(NIL_BASIC_BLOCK *ifTrueBasicBlock,
                            NIL_BASIC_BLOCK *ifFalseBasicBlock, NIL_VALUE *cond,
                            NIL_BASIC_BLOCK *block)
{
    assert(ifTrueBasicBlock);
    NIL_CONTEXT *context = nilGetBlockContext(ifTrueBasicBlock);

    NIL_BRANCH_INST *br = (NIL_BRANCH_INST *)ntMalloc(sizeof(NIL_BRANCH_INST));

    // value
    br->name = NULL;
    br->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    br->dbgLoc = NULL;
    br->type = nilGetVoidType(context);
    br->parent = NULL;

    // instruction
    br->opcode = NIL_TERM_BR;

    // binary operator
    br->ifTrue = ifTrueBasicBlock;
    br->ifFalse = ifFalseBasicBlock;
    br->condition = cond;

    insertInst(block, (NIL_VALUE *)br);

    return (NIL_VALUE *)br;
}

bool nilIsUnconditional(NIL_VALUE *branch)
{
    TO_BR(branch);
    return br->condition == NULL;
}

bool nilIsConditional(NIL_VALUE *branch)
{
    TO_BR(branch);
    return br->condition != NULL;
}

NIL_VALUE *nilGetCondition(NIL_VALUE *branchOrSelect)
{
    TO_INST(branchOrSelect);
    assert(inst->opcode == NIL_TERM_BR || inst->opcode == NIL_OTHER_SELECT);

    if (inst->opcode == NIL_TERM_BR)
    {
        TO_BR(inst);
        return br->condition;
    }
    else if (inst->opcode)
    {
        TO_SEL(inst);
        return sel->condition;
    }

    assert(0);
    return NULL;
}

void nilSetCondition(NIL_VALUE *branchOrSelect, NIL_VALUE *condition)
{
    TO_INST(branchOrSelect);
    assert(inst->opcode == NIL_TERM_BR || inst->opcode == NIL_OTHER_SELECT);

    NIL_TYPE *const conditionType = nilGetType(condition);
    NIL_CONTEXT *const context = nilGetTypeContext(conditionType);
    assert(conditionType == nilGetInt1Type(context));

    if (inst->opcode == NIL_TERM_BR)
    {
        TO_BR(inst);
        br->condition = condition;
    }
    else if (inst->opcode)
    {
        TO_SEL(inst);
        sel->condition = condition;
    }
}

size_t nilGetSucessorCount(NIL_VALUE *term)
{
    TO_INST(term);
    if (inst->opcode == NIL_TERM_BR)
    {
        if (nilIsUnconditional(term))
            return 1;
        else if (nilIsConditional(term))
            return 2;
        return 0;
    }
    return true;
}

void nilSetSuccessor(NIL_VALUE *branch, size_t index,
                     NIL_BASIC_BLOCK *newBasicBlockSucessor)
{
    TO_BR(branch);

    assert(!nilIsUnconditional(branch) || index == 0);
    assert(!nilIsConditional(branch) || index <= 1);

    switch (index)
    {
    case 0:
        br->ifTrue = newBasicBlockSucessor;
        break;
    case 1:
        br->ifFalse = newBasicBlockSucessor;
        break;
    default:
        assert(0);
        break;
    }
}

/********/
/* Call */
/********/

NIL_VALUE *nilCreateCall(NIL_TYPE *functionType, NIL_FUNCTION *function,
                         size_t argCount, NIL_VALUE **args, const char_t *name,
                         NIL_BASIC_BLOCK *block)
{
    assert(block);
    assert(functionType);
    assert(function);
    assert(argCount == 0 || args);
    assert(functionType->id == NIL_TYPE_FUNCTION);

    NIL_CONTEXT *const context = nilGetBlockContext(block);

    NIL_CALL_INST *call = (NIL_CALL_INST *)ntMalloc(sizeof(NIL_CALL_INST));

    NIL_TYPE *const resultType = ((NIL_FUNCTION_TYPE *)functionType)->result;
    const bool hasResult = !nilIsVoidType(resultType);

    // value
    call->name = hasResult ? nilGetPrefixedId(context, name) : NULL;
    call->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    call->dbgLoc = NULL;
    call->type = resultType;
    call->parent = NULL;

    // instruction
    call->opcode = NIL_OTHER_CALL;

    // call
    call->functionType = (NIL_FUNCTION_TYPE *)functionType;
    call->function = function;
    call->numArgs = argCount;
    call->arguments = args;

    insertInst(block, (NIL_VALUE *)call);
    return (NIL_VALUE *)call;
}

NIL_TYPE *nilGetCallFunctionType(NIL_VALUE *_call)
{
    TO_CALL(_call);
    return (NIL_TYPE *)call->functionType;
}

NIL_FUNCTION *nilGetCaller(NIL_VALUE *_call)
{
    TO_CALL(_call);
    return call->function;
}

void nilSetCaller(NIL_VALUE *_call, NIL_TYPE *functionType,
                  NIL_FUNCTION *function)
{
    assert(functionType);
    assert(functionType->id == NIL_TYPE_FUNCTION);
    assert(function);
    TO_CALL(_call);

    call->functionType = (NIL_FUNCTION_TYPE *)functionType;
    call->type = ((NIL_FUNCTION_TYPE *)functionType)->result;
    call->function = function;
}

size_t nilGetArgSize(NIL_VALUE *_call)
{
    TO_CALL(_call);
    return call->numArgs;
}

NIL_VALUE *nilGetArgOperand(NIL_VALUE *_call, size_t index)
{
    TO_CALL(_call);
    return call->arguments[index];
}

void nilSetArgOperand(NIL_VALUE *_call, size_t index, NIL_VALUE *value)
{
    assert(value);
    TO_CALL(_call);
    call->arguments[index] = value;
}

/****************/
/* Compare(CMP) */
/****************/

NIL_CMP_PREDICATE nilInversePredicate(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_EQ:
        return NIL_FCMP_NE;
    case NIL_FCMP_GT:
        return NIL_FCMP_LE;
    case NIL_FCMP_GE:
        return NIL_FCMP_LT;
    case NIL_FCMP_LT:
        return NIL_FCMP_GE;
    case NIL_FCMP_LE:
        return NIL_FCMP_GT;
    case NIL_FCMP_NE:
        return NIL_FCMP_EQ;
    case NIL_FCMP_OR:
        return NIL_FCMP_UO;
    case NIL_FCMP_UO:
        return NIL_FCMP_OR;

    case NIL_ICMP_EQ:
        return NIL_ICMP_NE;
    case NIL_ICMP_NE:
        return NIL_ICMP_EQ;
    case NIL_ICMP_UGT:
        return NIL_ICMP_ULE;
    case NIL_ICMP_UGE:
        return NIL_ICMP_ULT;
    case NIL_ICMP_ULT:
        return NIL_ICMP_UGE;
    case NIL_ICMP_ULE:
        return NIL_ICMP_UGT;

    case NIL_ICMP_SGT:
        return NIL_ICMP_SLE;
    case NIL_ICMP_SGE:
        return NIL_ICMP_SLT;
    case NIL_ICMP_SLT:
        return NIL_ICMP_SGE;
    case NIL_ICMP_SLE:
        return NIL_ICMP_SGT;
    default:
        assert(0 && "Unknow predicate.");
        return 0;
    }
}

NIL_CMP_PREDICATE nilStrictPredicate(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_GE:
        return NIL_FCMP_GT;
    case NIL_FCMP_LE:
        return NIL_FCMP_LT;

    case NIL_ICMP_UGE:
        return NIL_ICMP_UGT;
    case NIL_ICMP_ULE:
        return NIL_ICMP_ULT;

    case NIL_ICMP_SGE:
        return NIL_ICMP_SGT;
    case NIL_ICMP_SLE:
        return NIL_ICMP_SLT;
    default:
        return predicate;
    }
}

NIL_CMP_PREDICATE nilNonStrictPredicate(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_GT:
        return NIL_FCMP_GE;
    case NIL_FCMP_LT:
        return NIL_FCMP_LE;

    case NIL_ICMP_UGT:
        return NIL_ICMP_UGE;
    case NIL_ICMP_ULT:
        return NIL_ICMP_ULE;

    case NIL_ICMP_SGT:
        return NIL_ICMP_SGE;
    case NIL_ICMP_SLT:
        return NIL_ICMP_SLE;
    default:
        return predicate;
    }
}

NIL_CMP_PREDICATE nilSignedPredicate(NIL_CMP_PREDICATE predicate)
{
    assert(nilIsUnsigned(predicate) && "only call with unsigned predicates!");

    switch (predicate)
    {
    case NIL_ICMP_ULT:
        return NIL_ICMP_SLT;
    case NIL_ICMP_ULE:
        return NIL_ICMP_SLE;
    case NIL_ICMP_UGT:
        return NIL_ICMP_SGT;
    case NIL_ICMP_UGE:
        return NIL_ICMP_SGE;
    default:
        assert(0);
        return 0;
    }
}

NIL_CMP_PREDICATE nilUnsignedPredicate(NIL_CMP_PREDICATE predicate)
{
    assert(nilIsSigned(predicate) && "only call with signed predicates!");

    switch (predicate)
    {
    case NIL_ICMP_SLT:
        return NIL_ICMP_ULT;
    case NIL_ICMP_SLE:
        return NIL_ICMP_ULE;
    case NIL_ICMP_SGT:
        return NIL_ICMP_UGT;
    case NIL_ICMP_SGE:
        return NIL_ICMP_UGE;
    default:
        assert(0);
        return 0;
    }
}

bool nilIsIntPredicate(NIL_CMP_PREDICATE predicate)
{
    return predicate >= NIL_ICMP_FIRST && predicate <= NIL_ICMP_LAST;
}

bool nilIsFPPredicate(NIL_CMP_PREDICATE predicate)
{
    return predicate >= NIL_FCMP_FIRST && predicate <= NIL_FCMP_LAST;
}

bool nilIsStrictPredicate(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_GT:
    case NIL_FCMP_LT:
    case NIL_ICMP_UGT:
    case NIL_ICMP_ULT:
    case NIL_ICMP_SGT:
    case NIL_ICMP_SLT:
        return true;
    default:
        return false;
    }
}

bool nilIsEquality(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_ICMP_EQ:
    case NIL_ICMP_NE:
    case NIL_FCMP_EQ:
    case NIL_FCMP_NE:
        return true;
    default:
        return false;
    }
}

bool nilIsRelational(NIL_CMP_PREDICATE predicate)
{
    return !nilIsEquality(predicate);
}

bool nilIsSigned(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_ICMP_SGT:
    case NIL_ICMP_SGE:
    case NIL_ICMP_SLT:
    case NIL_ICMP_SLE:
        return true;
    default:
        return false;
    }
}

bool nilIsUnsigned(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_ICMP_UGT:
    case NIL_ICMP_UGE:
    case NIL_ICMP_ULT:
    case NIL_ICMP_ULE:
        return true;
    default:
        return false;
    }
}

bool nilIsTrueWhenEqual(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_EQ:
    case NIL_FCMP_GE:
    case NIL_FCMP_LE:

    case NIL_ICMP_EQ:

    case NIL_ICMP_UGE:
    case NIL_ICMP_ULE:

    case NIL_ICMP_SGE:
    case NIL_ICMP_SLE:
        return true;
    default:
        return false;
    }
}

bool nilIsFalseWhenEqual(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_NE:
    case NIL_FCMP_GT:
    case NIL_FCMP_LT:

    case NIL_ICMP_NE:

    case NIL_ICMP_UGT:
    case NIL_ICMP_ULT:

    case NIL_ICMP_SGT:
    case NIL_ICMP_SLT:
        return true;
    default:
        return false;
    }
}

bool nilIsImpliedTrueByMatchingCmp(NIL_CMP_PREDICATE predicate1,
                                   NIL_CMP_PREDICATE predicate2)
{
    if (predicate1 == predicate2)
        return true;

    switch (predicate1)
    {
    case NIL_ICMP_EQ:
        switch (predicate2)
        {
        case NIL_ICMP_UGE:
        case NIL_ICMP_ULE:
        case NIL_ICMP_SGE:
        case NIL_ICMP_SLE:
            // A == B implies any non-strict A (>=u, <=u, >=s, <=s) B are true.
            return true;
        default:
            return false;
        }
    case NIL_ICMP_UGT:
        // A >u B implies A !=u B and A >=u B are true.
        return predicate2 == NIL_ICMP_NE || predicate2 == NIL_ICMP_UGE;
    case NIL_ICMP_ULT:
        // A <u B implies A !=u B and A <=u B are true.
        return predicate2 == NIL_ICMP_NE || predicate2 == NIL_ICMP_ULE;
    case NIL_ICMP_SGT:
        // A >s B implies A !=s B and A >=s B are true.
        return predicate2 == NIL_ICMP_NE || predicate2 == NIL_ICMP_SGE;
    case NIL_ICMP_SLT:
        // A <u B implies A !=u B and A <=u B are true.
        return predicate2 == NIL_ICMP_NE || predicate2 == NIL_ICMP_SLE;
    default:
        return false;
    }
}

bool nilIsImpliedFalseByMatchingCmp(NIL_CMP_PREDICATE predicate1,
                                    NIL_CMP_PREDICATE predicate2)
{
    return nilIsImpliedTrueByMatchingCmp(predicate1,
                                         nilInversePredicate(predicate2));
}

const char *nilGetPredicateName(NIL_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIL_FCMP_EQ:
        return "eq";
    case NIL_FCMP_GT:
        return "gt";
    case NIL_FCMP_GE:
        return "ge";
    case NIL_FCMP_LT:
        return "lt";
    case NIL_FCMP_LE:
        return "le";
    case NIL_FCMP_NE:
        return "ne";
    case NIL_FCMP_OR:
        return "or";
    case NIL_FCMP_UO:
        return "uo";

    case NIL_ICMP_EQ:
        return "eq";
    case NIL_ICMP_NE:
        return "ne";

    case NIL_ICMP_UGT:
        return "ugt";
    case NIL_ICMP_UGE:
        return "uge";
    case NIL_ICMP_ULT:
        return "ult";
    case NIL_ICMP_ULE:
        return "ule";

    case NIL_ICMP_SGT:
        return "sgt";
    case NIL_ICMP_SGE:
        return "sge";
    case NIL_ICMP_SLT:
        return "slt";
    case NIL_ICMP_SLE:
        return "sle";
    default:
        return "unknown";
    }
}

NIL_CMP_PREDICATE nilGetPredicate(NIL_VALUE *_cmp)
{
    TO_CMP(_cmp);
    return cmp->predicate;
}

NIL_CMP_PREDICATE nilGetInversePredicate(NIL_VALUE *_cmp)
{
    TO_CMP(_cmp);
    return nilInversePredicate(cmp->predicate);
}

NIL_VALUE *nilCreateCmp(NIL_CMP_PREDICATE predicate, NIL_VALUE *source1,
                        NIL_VALUE *source2, const char_t *name,
                        NIL_BASIC_BLOCK *block)
{
    assert(name);
    assert(block);

    NIL_CONTEXT *const context = nilGetBlockContext(block);

    NIL_CMP_INST *cmp = (NIL_CMP_INST *)ntMalloc(sizeof(NIL_CMP_INST));

    // value
    cmp->name = nilGetPrefixedId(context, name);
    cmp->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    cmp->dbgLoc = NULL;
    cmp->type = nilGetInt1Type(context);
    cmp->parent = NULL;

    // instruction
    cmp->opcode = NIL_OTHER_CMP;

    // cmp
    cmp->source1 = source1;
    cmp->source2 = source2;
    cmp->predicate = predicate;

    insertInst(block, (NIL_VALUE *)cmp);
    return (NIL_VALUE *)cmp;
}

/************/
/* PHI Node */
/************/

NIL_VALUE *nilCreatePhi(NIL_TYPE *type, const char_t *name,
                        NIL_BASIC_BLOCK *block)
{
    assert(type);
    assert(name);
    assert(block);

    NIL_CONTEXT *const context = nilGetBlockContext(block);

    NIL_PHI_NODE *phi = (NIL_PHI_NODE *)ntMalloc(sizeof(NIL_PHI_NODE));

    // value
    phi->name = nilGetPrefixedId(context, name);
    phi->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    phi->dbgLoc = NULL;
    phi->type = type;
    phi->parent = NULL;

    // instruction
    phi->opcode = NIL_OTHER_PHI;

    // phi
    listInit(&phi->list);

    insertInst(block, (NIL_VALUE *)phi);
    return (NIL_VALUE *)phi;
}

size_t nilGetNumIncomingValues(NIL_VALUE *_phi)
{
    TO_PHI(_phi);
    return phi->list.count;
}

NIL_VALUE *nilGetIncomingValue(NIL_VALUE *_phi, size_t i)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    return phi->incomingValues[i]->value;
}

void nilSetIncomingValue(NIL_VALUE *_phi, size_t i, NIL_VALUE *value)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    phi->incomingValues[i]->value = value;
}

NIL_BASIC_BLOCK *nilGetIncomingBlock(NIL_VALUE *_phi, size_t i)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    return phi->incomingValues[i]->block;
}

void nilSetIncomingBlock(NIL_VALUE *_phi, size_t i, NIL_BASIC_BLOCK *block)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    phi->incomingValues[i]->block = block;
}

void nilAddIncoming(NIL_VALUE *_phi, NIL_VALUE *value, NIL_BASIC_BLOCK *block)
{
    assert(value);
    assert(block);

    TO_PHI(_phi);

    NIL_INCOMING_VALUE *iv =
        (NIL_INCOMING_VALUE *)ntMalloc(sizeof(NIL_INCOMING_VALUE));
    iv->value = value;
    iv->block = block;

    listAdd(&phi->list, iv);
}

NIL_VALUE *nilRemoveIncomingValue(NIL_VALUE *_phi, size_t i)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    NIL_VALUE *value = phi->incomingValues[i]->value;
    listRemove(&phi->list, i);
    return value;
}

NIL_VALUE *nilRemoveIncomingBlock(NIL_VALUE *phi, NIL_BASIC_BLOCK *block)
{
    const size_t index = nilGetPhiBasicBlockIndex(phi, block);
    return nilRemoveIncomingValue(phi, index);
}

size_t nilGetPhiBasicBlockIndex(NIL_VALUE *_phi, NIL_BASIC_BLOCK *block)
{
    TO_PHI(_phi);
    assert(block);
    for (size_t i = 0; i < phi->list.count; ++i)
    {
        NIL_INCOMING_VALUE *iv = phi->incomingValues[i];
        if (iv->block == block)
            return i;
    }
    assert(0);
    return -1;
}

NIL_VALUE *nilGetIncomingValueForBlock(NIL_VALUE *_phi, NIL_BASIC_BLOCK *block)
{
    TO_PHI(_phi);
    assert(block);
    for (size_t i = 0; i < phi->list.count; ++i)
    {
        NIL_INCOMING_VALUE *iv = phi->incomingValues[i];
        if (iv->block == block)
            return iv->value;
    }
    return NULL;
}

NIL_VALUE *nilPhiHasConstantValue(NIL_VALUE *_phi)
{
    TO_PHI(_phi);
    assert(phi->list.count > 0);

    NIL_VALUE *constant = phi->incomingValues[0]->value;
    for (size_t i = 1, count = phi->list.count; i != count; ++i)
    {
        NIL_VALUE *const v = phi->incomingValues[i]->value;
        if (v == constant || v == _phi)
            continue;
        if (constant != _phi)
            return NULL;
        constant = v;
    }
    if (constant == _phi)
        return NIL_VALUE_UNDEF;
    return constant;
}

bool nilPhiHasConstantOrUndefValue(NIL_VALUE *_phi)
{
    TO_PHI(_phi);
    assert(phi->list.count > 0);

    NIL_VALUE *constant = NULL;
    for (size_t i = 0, count = phi->list.count; i != count; ++i)
    {
        NIL_VALUE *const v = phi->incomingValues[i]->value;
        if (v == _phi || v == NIL_VALUE_UNDEF)
            continue;
        if (constant && constant != v)
            return false;
        constant = v;
    }
    return true;
}

bool nilPhiIsComplete(NIL_VALUE *_phi)
{
    TO_PHI(_phi);

    NIL_BASIC_BLOCK *parent = phi->parent;
    assert(parent);

    const size_t count = nilGetPredecessorCount(parent);
    const size_t incomingCount = phi->list.count;

    if (count < incomingCount)
        return false; // missing incoming values

    for (size_t i = 0; i < count; ++i)
    {
        NIL_BASIC_BLOCK *const predecessor = nilGetPredecessor(parent, i);

        bool find = false;
        for (size_t j = 0; j < incomingCount; ++j)
        {
            NIL_BASIC_BLOCK *incomingBlock = phi->incomingValues[j]->block;
            if (incomingBlock == predecessor)
            {
                find = true;
                break;
            }
        }

        if (!find)
            // incoming value is missing for at least one predecessor block
            return false;
    }

    return true;
}

/**********/
/* Return */
/**********/

NIL_VALUE *nilCreateReturn(NIL_VALUE *returnValue, NIL_BASIC_BLOCK *block)
{
    assert(block);

    NIL_RETURN_INST *ret = (NIL_RETURN_INST *)ntMalloc(sizeof(NIL_RETURN_INST));

    NIL_CONTEXT *context = nilGetBlockContext(block);

    // value
    ret->name = NULL;
    ret->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    ret->dbgLoc = NULL;
    ret->type = nilGetVoidType(context);
    ret->parent = NULL;

    // instruction
    ret->opcode = NIL_TERM_RET;

    // ret
    ret->retValue = returnValue;

    insertInst(block, (NIL_VALUE *)ret);
    return (NIL_VALUE *)ret;
}

NIL_VALUE *nilGetReturnValue(NIL_VALUE *_ret)
{
    TO_RET(_ret);
    return ret->retValue;
}

/**********/
/* Select */
/**********/

NIL_VALUE *nilCreateSelect(NIL_VALUE *condition, NIL_VALUE *valueWhenTrue,
                           NIL_VALUE *valueWhenFalse, const char_t *name,
                           NIL_BASIC_BLOCK *block)
{
    assert(block);
    assert(condition);
    assert(valueWhenTrue);
    assert(valueWhenFalse);
    assert(name);

    NIL_CONTEXT *const context = nilGetBlockContext(block);

    assert(nilGetInt1Type(context) == nilGetType(condition));
    assert(nilGetVoidType(context) != nilGetType(valueWhenTrue));
    assert(nilGetVoidType(context) != nilGetType(valueWhenFalse));
    assert(nilGetType(valueWhenTrue) == nilGetType(valueWhenFalse));

    NIL_SELECT_INST *sel = (NIL_SELECT_INST *)ntMalloc(sizeof(NIL_SELECT_INST));

    // value
    sel->name = nilGetPrefixedId(context, name);
    sel->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    sel->dbgLoc = NULL;
    sel->type = nilGetType(valueWhenTrue);
    sel->parent = NULL;

    // instruction
    sel->opcode = NIL_OTHER_SELECT;

    // sel
    sel->condition = condition;
    sel->trueValue = valueWhenTrue;
    sel->falseValue = valueWhenFalse;

    insertInst(block, (NIL_VALUE *)sel);
    return (NIL_VALUE *)sel;
}

// see nilGetCondition in branch
// NIL_VALUE *nilGetCondition(NIL_VALUE *select)

NIL_VALUE *nilGetTrueValue(NIL_VALUE *select)
{
    TO_SEL(select);
    return sel->trueValue;
}

NIL_VALUE *nilGetFalseValue(NIL_VALUE *select)
{
    TO_SEL(select);
    return sel->falseValue;
}

// see nilSetCondition in branch
// void nilSetCondition(NIL_VALUE *select, NIL_VALUE *condition)

void nilSetTrueValue(NIL_VALUE *select, NIL_VALUE *value)
{
    TO_SEL(select);
    sel->trueValue = value;
}

void nilSetFalseValue(NIL_VALUE *select, NIL_VALUE *value)
{
    TO_SEL(select);
    sel->falseValue = value;
}

void nilSwapValues(NIL_VALUE *select)
{
    TO_SEL(select);
    NIL_VALUE *const tmp = sel->trueValue;
    sel->trueValue = sel->falseValue;
    sel->falseValue = tmp;
}

const char *nilAreSelectInvalidOperands(NIL_VALUE *condition,
                                        NIL_VALUE *trueValue,
                                        NIL_VALUE *falseValue)
{
    assert(condition);
    assert(trueValue);
    assert(falseValue);

    if (nilGetType(trueValue) == nilGetType(falseValue))
        return "both values must have same type";

    NIL_TYPE *conditionType = nilGetType(condition);
    NIL_CONTEXT *const context = nilGetTypeContext(conditionType);
    if (conditionType != nilGetInt1Type(context))
        return "codition must be i1";

    return NULL;
}

/*********/
/* Store */
/*********/

/**
 * @brief Construct a store instruction, given value and pointer. Also
 * automatically insert this instruction to the end of the BasicBlock specified.
 *
 * @param value Value to store.
 * @param ptr Pointer to memory.
 * @param basicBlock Basic block to insert.
 * @return NIL_VALUE* that represents the instruction.
 */
NIL_VALUE *nilCreateStore(NIL_VALUE *value, NIL_VALUE *ptr,
                          NIL_BASIC_BLOCK *block)
{
    assert(block);
    assert(value);
    assert(ptr);

    NIL_CONTEXT *context = nilGetBlockContext(block);

    NIL_TYPE *ptrType = nilGetType(ptr);
    assert(nilIsPointerType(ptrType));
    // assert(nilGetPointeeType(ptrType) == nilGetType(value));

    NIL_STORE_INST *str = (NIL_STORE_INST *)ntMalloc(sizeof(NIL_STORE_INST));
    // value
    str->name = NULL;
    str->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    str->dbgLoc = NULL;
    str->type = nilGetVoidType(context);
    str->parent = NULL;

    // instruction
    str->opcode = NIL_OTHER_MEMORY_STORE;

    // sel
    str->source = value;
    str->ptr = ptr;

    insertInst(block, (NIL_VALUE *)str);
    return (NIL_VALUE *)str;
}

NIL_VALUE *nilGetPointerOperand(NIL_VALUE *store)
{
    TO_STR(store);
    return str->ptr;
}

NIL_TYPE *nilGetPointerOperandType(NIL_VALUE *store)
{
    TO_STR(store);
    return nilGetType(str->ptr);
}

NIL_VALUE *nilGetValueOperand(NIL_VALUE *store)
{
    TO_STR(store);
    return str->source;
}

/**********/
/* Unary */
/**********/

NIL_VALUE *nilCreateUnaryInst(NIL_OPCODE unaryOp, NIL_TYPE *valueType,
                              NIL_VALUE *value, const char_t *name,
                              NIL_BASIC_BLOCK *block)
{
    assert(nilIsUnaryOp(unaryOp) && "operation is not unary!");
    assert(name);
    assert(block);
    assert(valueType);

    NIL_CONTEXT *context = nilGetBlockContext(block);

    NIL_UNARY_OPERATOR *una =
        (NIL_UNARY_OPERATOR *)ntMalloc(sizeof(NIL_UNARY_OPERATOR));
    // value
    una->name = nilGetPrefixedId(context, name);
    una->valueType = NIL_VALUE_TYPE_INSTRUCTION;
    una->dbgLoc = NULL;
    una->type = valueType;
    una->parent = NULL;

    // instruction
    una->opcode = unaryOp;

    // sel
    una->source = value;

    insertInst(block, (NIL_VALUE *)una);
    return (NIL_VALUE *)una;
}

NIL_VALUE *nilGetUnaryValueOperand(NIL_VALUE *unary)
{
    TO_UNA(unary);
    return una->source;
}

NIL_TYPE *nilGetUnaryTypeOperand(NIL_VALUE *unary)
{
    TO_UNA(unary);
    return nilGetType(una->source);
}
