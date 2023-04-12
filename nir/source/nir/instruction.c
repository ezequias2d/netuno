
#include "netuno/nir/instruction.h"
#include "netuno/memory.h"
#include "netuno/nir/basic_block.h"
#include "netuno/nir/constant.h"
#include "netuno/nir/context.h"
#include "netuno/nir/type.h"
#include "netuno/nir/value.h"
#include "netuno/string.h"
#include "nir/pbasic_block.h"
#include "nir/pcontext.h"
#include "nir/plist.h"
#include "pinstruction.h"
#include "ptype.h"
#include <assert.h>

const char *nirGetOpcodeName(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case (NIR_TERM_RET):
        return "ret";
    case (NIR_TERM_BR):
        return "br";

    // unary operators
    case (NIR_UNARY_FNEG):
        return "fneg";
    case (NIR_UNARY_MEMORY_ALLOCA):
        return "alloca";
    case (NIR_UNARY_MEMORY_LOAD):
        return "load";
    case (NIR_UNARY_CAST_TRUNC):
        return "trunc";
    case (NIR_UNARY_CAST_ZEXT):
        return "zext";
    case (NIR_UNARY_CAST_SEXT):
        return "sext";
    case (NIR_UNARY_CAST_FP_TO_UI):
        return "fp_to_ui";
    case (NIR_UNARY_CAST_FP_TO_SI):
        return "fp_to_si";
    case (NIR_UNARY_CAST_UI_TO_FP):
        return "ui_to_fp";
    case (NIR_UNARY_CAST_SI_TO_FP):
        return "si_to_fp";
    case (NIR_UNARY_CAST_FP_TRUNC):
        return "fp_trunc";
    case (NIR_UNARY_CAST_PTR_TO_INT):
        return "ptr_to_int";
    case (NIR_UNARY_CAST_INT_TO_PTR):
        return "int_to_ptr";
    case (NIR_UNARY_CAST_BIT_CAST):
        return "bit_cast";

    // binary operators
    case (NIR_BINARY_OP_ADD):
        return "add";
    case (NIR_BINARY_OP_FADD):
        return "fadd";
    case (NIR_BINARY_OP_SUB):
        return "sub";
    case (NIR_BINARY_OP_FSUB):
        return "fsub";
    case (NIR_BINARY_OP_MUL):
        return "mul";
    case (NIR_BINARY_OP_FMUL):
        return "fmul";
    case (NIR_BINARY_OP_UDIV):
        return "udiv";
    case (NIR_BINARY_OP_SDIV):
        return "sdiv";
    case (NIR_BINARY_OP_FDIV):
        return "fdiv";
    case (NIR_BINARY_OP_UREM):
        return "urem";
    case (NIR_BINARY_OP_SREM):
        return "srem";
    case (NIR_BINARY_OP_FREM):
        return "frem";

    // logical operators
    case (NIR_BINARY_OP_SHL):
        return "shl";
    case (NIR_BINARY_OP_SHR):
        return "shr";
    case (NIR_BINARY_OP_ASR):
        return "asr";
    case (NIR_BINARY_OP_AND):
        return "and";
    case (NIR_BINARY_OP_OR):
        return "or";
    case (NIR_BINARY_OP_XOR):
        return "xor";

    // others
    case (NIR_GET_ELEMENT_PTR):
        return "get_element_ptr";
    case (NIR_EXTRACT_VALUE):
        return "extract_value";
    case (NIR_OTHER_CMP):
        return "cmp";
    case (NIR_OTHER_PHI):
        return "phi";
    case (NIR_OTHER_CALL):
        return "call";
    case (NIR_OTHER_SELECT):
        return "select";
    case NIR_OTHER_MEMORY_STORE:
        return "store";

    case (NIR_OTHER_INSERT_VALUE):
        return "insert_value";

    default:
        assert(false);
        return NULL;
    }
}

bool nirIsTermiantor(NIR_OPCODE opcode)
{
    return opcode >= NIR_TERM_OPS_BEGIN && opcode < NIR_TERM_OPS_END;
}

bool nirIsUnaryOp(NIR_OPCODE opcode)
{
    return opcode >= NIR_UNARY_OPS_BEGIN && opcode < NIR_UNARY_OPS_END;
}

bool nirIsBinaryOp(NIR_OPCODE opcode)
{
    return opcode >= NIR_BINARY_OPS_BEGIN && opcode < NIR_BINARY_OPS_END;
}

bool nirIsIntDivRem(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_UDIV:
    case NIR_BINARY_OP_SDIV:
    case NIR_BINARY_OP_FDIV:
    case NIR_BINARY_OP_UREM:
    case NIR_BINARY_OP_SREM:
    case NIR_BINARY_OP_FREM:
        return true;
    default:
        return false;
    }
}

bool nirIsShift(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_SHL:
    case NIR_BINARY_OP_SHR:
    case NIR_BINARY_OP_ASR:
        return true;
    default:
        return false;
    }
}

bool nirIsLogicalShift(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_SHL:
    case NIR_BINARY_OP_SHR:
        return true;
    default:
        return false;
    }
}

bool nirIsArithmeticShift(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_SHL:
    case NIR_BINARY_OP_ASR:
        return true;
    default:
        return false;
    }
}

bool nirIsBitwiseLogicOp(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_AND:
    case NIR_BINARY_OP_OR:
    case NIR_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

bool nirIsCast(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_UNARY_CAST_TRUNC:
    case NIR_UNARY_CAST_ZEXT:
    case NIR_UNARY_CAST_SEXT:
    case NIR_UNARY_CAST_FP_TO_UI:
    case NIR_UNARY_CAST_FP_TO_SI:
    case NIR_UNARY_CAST_UI_TO_FP:
    case NIR_UNARY_CAST_SI_TO_FP:
    case NIR_UNARY_CAST_FP_TRUNC:
    case NIR_UNARY_CAST_PTR_TO_INT:
    case NIR_UNARY_CAST_INT_TO_PTR:
    case NIR_UNARY_CAST_BIT_CAST:
        return true;
    default:
        return false;
    }
}

bool nirIsAssociative(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_ADD:
    case NIR_BINARY_OP_MUL:
    case NIR_BINARY_OP_AND:
    case NIR_BINARY_OP_OR:
    case NIR_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

bool nirIsCommutative(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_ADD:
    case NIR_BINARY_OP_FADD:
    case NIR_BINARY_OP_MUL:
    case NIR_BINARY_OP_FMUL:
    case NIR_BINARY_OP_AND:
    case NIR_BINARY_OP_OR:
    case NIR_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

bool nirIsIdempotent(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_AND:
    case NIR_BINARY_OP_OR:
        return true;
    default:
        return false;
    }
}

bool nirIsNilpotent(NIR_OPCODE opcode)
{
    switch (opcode)
    {
    case NIR_BINARY_OP_XOR:
        return true;
    default:
        return false;
    }
}

NIR_OPCODE nirGetOpcode(NIR_VALUE *instruction)
{
    TO_INST(instruction);
    return inst->opcode;
}

/**********/
/* Binary */
/**********/

NIR_VALUE *nirCreateBinary(NIR_OPCODE op, NIR_VALUE *source1,
                           NIR_VALUE *source2, const char_t *name,
                           NIR_BASIC_BLOCK *block)
{
    assert(nirIsBinaryOp(op));
    // assert(!source1 || !source2 ||
    //        nirGetType(source1) == nirGetType(source2) &&
    //            "operand1 and operand2 must has same type");

    NIR_CONTEXT *const context = nirGetBlockContext(block);

    NIR_BINARY_OPERATOR *const bin = ntMalloc(sizeof(NIR_BINARY_OPERATOR));

    // value
    bin->name = nirGetPrefixedId(context, name);
    bin->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    bin->dbgLoc = NULL;
    bin->type = source1 ? nirGetType(source1) : NULL;
    bin->parent = NULL;

    // instruction
    bin->opcode = op;

    // binary operator
    bin->source1 = source1;
    bin->source2 = source2;

    insertInst(block, (NIR_VALUE *)bin);

    return (NIR_VALUE *)bin;
}

NIR_VALUE *nirCreateNeg(NIR_VALUE *source, const char_t *name,
                        NIR_BASIC_BLOCK *block)
{
    NIR_VALUE *const zero = nirGetInt(nirGetType(source), 0, false);
    return nirCreateBinary(NIR_BINARY_OP_SUB, zero, source, name, block);
}

NIR_VALUE *nirCreateNot(NIR_VALUE *source, const char_t *name,
                        NIR_BASIC_BLOCK *block)
{
    NIR_VALUE *const ones = nirGetIntAllOnes(nirGetType(source));
    return nirCreateBinary(NIR_BINARY_OP_XOR, ones, source, name, block);
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
 * @return NIR_VALUE* Value that represents the instruction.
 */
NIR_VALUE *nirCreateBranch1(NIR_BASIC_BLOCK *destBasicBlock,
                            NIR_BASIC_BLOCK *block)
{
    return nirCreateBranch2(destBasicBlock, NULL, NULL, block);
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
 * @return NIR_VALUE* Value that represents the instruction.
 */
NIR_VALUE *nirCreateBranch2(NIR_BASIC_BLOCK *ifTrueBasicBlock,
                            NIR_BASIC_BLOCK *ifFalseBasicBlock, NIR_VALUE *cond,
                            NIR_BASIC_BLOCK *block)
{
    assert(ifTrueBasicBlock);
    NIR_CONTEXT *context = nirGetBlockContext(ifTrueBasicBlock);

    NIR_BRANCH_INST *br = (NIR_BRANCH_INST *)ntMalloc(sizeof(NIR_BRANCH_INST));

    // value
    br->name = NULL;
    br->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    br->dbgLoc = NULL;
    br->type = nirGetVoidType(context);
    br->parent = NULL;

    // instruction
    br->opcode = NIR_TERM_BR;

    // binary operator
    br->ifTrue = ifTrueBasicBlock;
    br->ifFalse = ifFalseBasicBlock;
    br->condition = cond;

    insertInst(block, (NIR_VALUE *)br);

    return (NIR_VALUE *)br;
}

bool nirIsUnconditional(NIR_VALUE *branch)
{
    TO_BR(branch);
    return br->condition == NULL;
}

bool nirIsConditional(NIR_VALUE *branch)
{
    TO_BR(branch);
    return br->condition != NULL;
}

NIR_VALUE *nirGetCondition(NIR_VALUE *branchOrSelect)
{
    TO_INST(branchOrSelect);
    assert(inst->opcode == NIR_TERM_BR || inst->opcode == NIR_OTHER_SELECT);

    if (inst->opcode == NIR_TERM_BR)
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

void nirSetCondition(NIR_VALUE *branchOrSelect, NIR_VALUE *condition)
{
    TO_INST(branchOrSelect);
    assert(inst->opcode == NIR_TERM_BR || inst->opcode == NIR_OTHER_SELECT);

    NIR_TYPE *const conditionType = nirGetType(condition);
    NIR_CONTEXT *const context = nirGetTypeContext(conditionType);
    assert(conditionType == nirGetInt1Type(context));

    if (inst->opcode == NIR_TERM_BR)
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

size_t nirGetSucessorCount(NIR_VALUE *term)
{
    TO_INST(term);
    if (inst->opcode == NIR_TERM_BR)
    {
        if (nirIsUnconditional(term))
            return 1;
        else if (nirIsConditional(term))
            return 2;
        return 0;
    }
    return true;
}

void nirSetSuccessor(NIR_VALUE *branch, size_t index,
                     NIR_BASIC_BLOCK *newBasicBlockSucessor)
{
    TO_BR(branch);

    assert(!nirIsUnconditional(branch) || index == 0);
    assert(!nirIsConditional(branch) || index <= 1);

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

NIR_VALUE *nirCreateCall(NIR_TYPE *functionType, NIR_FUNCTION *function,
                         size_t argCount, NIR_VALUE **args, const char_t *name,
                         NIR_BASIC_BLOCK *block)
{
    assert(block);
    assert(functionType);
    assert(function);
    assert(argCount == 0 || args);
    assert(functionType->id == NIR_TYPE_FUNCTION);

    NIR_CONTEXT *const context = nirGetBlockContext(block);

    NIR_CALL_INST *call = (NIR_CALL_INST *)ntMalloc(sizeof(NIR_CALL_INST));

    NIR_TYPE *const resultType = ((NIR_FUNCTION_TYPE *)functionType)->result;
    const bool hasResult = !nirIsVoidType(resultType);

    // value
    call->name = hasResult ? nirGetPrefixedId(context, name) : NULL;
    call->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    call->dbgLoc = NULL;
    call->type = resultType;
    call->parent = NULL;

    // instruction
    call->opcode = NIR_OTHER_CALL;

    // call
    call->functionType = (NIR_FUNCTION_TYPE *)functionType;
    call->function = function;
    call->numArgs = argCount;
    call->arguments = args;

    insertInst(block, (NIR_VALUE *)call);
    return (NIR_VALUE *)call;
}

NIR_TYPE *nirGetCallFunctionType(NIR_VALUE *_call)
{
    TO_CALL(_call);
    return (NIR_TYPE *)call->functionType;
}

NIR_FUNCTION *nirGetCaller(NIR_VALUE *_call)
{
    TO_CALL(_call);
    return call->function;
}

void nirSetCaller(NIR_VALUE *_call, NIR_TYPE *functionType,
                  NIR_FUNCTION *function)
{
    assert(functionType);
    assert(functionType->id == NIR_TYPE_FUNCTION);
    assert(function);
    TO_CALL(_call);

    call->functionType = (NIR_FUNCTION_TYPE *)functionType;
    call->type = ((NIR_FUNCTION_TYPE *)functionType)->result;
    call->function = function;
}

size_t nirGetArgSize(NIR_VALUE *_call)
{
    TO_CALL(_call);
    return call->numArgs;
}

NIR_VALUE *nirGetArgOperand(NIR_VALUE *_call, size_t index)
{
    TO_CALL(_call);
    return call->arguments[index];
}

void nirSetArgOperand(NIR_VALUE *_call, size_t index, NIR_VALUE *value)
{
    assert(value);
    TO_CALL(_call);
    call->arguments[index] = value;
}

/****************/
/* Compare(CMP) */
/****************/

NIR_CMP_PREDICATE nirInversePredicate(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_EQ:
        return NIR_FCMP_NE;
    case NIR_FCMP_GT:
        return NIR_FCMP_LE;
    case NIR_FCMP_GE:
        return NIR_FCMP_LT;
    case NIR_FCMP_LT:
        return NIR_FCMP_GE;
    case NIR_FCMP_LE:
        return NIR_FCMP_GT;
    case NIR_FCMP_NE:
        return NIR_FCMP_EQ;
    case NIR_FCMP_OR:
        return NIR_FCMP_UO;
    case NIR_FCMP_UO:
        return NIR_FCMP_OR;

    case NIR_ICMP_EQ:
        return NIR_ICMP_NE;
    case NIR_ICMP_NE:
        return NIR_ICMP_EQ;
    case NIR_ICMP_UGT:
        return NIR_ICMP_ULE;
    case NIR_ICMP_UGE:
        return NIR_ICMP_ULT;
    case NIR_ICMP_ULT:
        return NIR_ICMP_UGE;
    case NIR_ICMP_ULE:
        return NIR_ICMP_UGT;

    case NIR_ICMP_SGT:
        return NIR_ICMP_SLE;
    case NIR_ICMP_SGE:
        return NIR_ICMP_SLT;
    case NIR_ICMP_SLT:
        return NIR_ICMP_SGE;
    case NIR_ICMP_SLE:
        return NIR_ICMP_SGT;
    default:
        assert(0 && "Unknow predicate.");
        return 0;
    }
}

NIR_CMP_PREDICATE nirStrictPredicate(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_GE:
        return NIR_FCMP_GT;
    case NIR_FCMP_LE:
        return NIR_FCMP_LT;

    case NIR_ICMP_UGE:
        return NIR_ICMP_UGT;
    case NIR_ICMP_ULE:
        return NIR_ICMP_ULT;

    case NIR_ICMP_SGE:
        return NIR_ICMP_SGT;
    case NIR_ICMP_SLE:
        return NIR_ICMP_SLT;
    default:
        return predicate;
    }
}

NIR_CMP_PREDICATE nirNonStrictPredicate(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_GT:
        return NIR_FCMP_GE;
    case NIR_FCMP_LT:
        return NIR_FCMP_LE;

    case NIR_ICMP_UGT:
        return NIR_ICMP_UGE;
    case NIR_ICMP_ULT:
        return NIR_ICMP_ULE;

    case NIR_ICMP_SGT:
        return NIR_ICMP_SGE;
    case NIR_ICMP_SLT:
        return NIR_ICMP_SLE;
    default:
        return predicate;
    }
}

NIR_CMP_PREDICATE nirSignedPredicate(NIR_CMP_PREDICATE predicate)
{
    assert(nirIsUnsigned(predicate) && "only call with unsigned predicates!");

    switch (predicate)
    {
    case NIR_ICMP_ULT:
        return NIR_ICMP_SLT;
    case NIR_ICMP_ULE:
        return NIR_ICMP_SLE;
    case NIR_ICMP_UGT:
        return NIR_ICMP_SGT;
    case NIR_ICMP_UGE:
        return NIR_ICMP_SGE;
    default:
        assert(0);
        return 0;
    }
}

NIR_CMP_PREDICATE nirUnsignedPredicate(NIR_CMP_PREDICATE predicate)
{
    assert(nirIsSigned(predicate) && "only call with signed predicates!");

    switch (predicate)
    {
    case NIR_ICMP_SLT:
        return NIR_ICMP_ULT;
    case NIR_ICMP_SLE:
        return NIR_ICMP_ULE;
    case NIR_ICMP_SGT:
        return NIR_ICMP_UGT;
    case NIR_ICMP_SGE:
        return NIR_ICMP_UGE;
    default:
        assert(0);
        return 0;
    }
}

bool nirIsIntPredicate(NIR_CMP_PREDICATE predicate)
{
    return predicate >= NIR_ICMP_FIRST && predicate <= NIR_ICMP_LAST;
}

bool nirIsFPPredicate(NIR_CMP_PREDICATE predicate)
{
    return predicate >= NIR_FCMP_FIRST && predicate <= NIR_FCMP_LAST;
}

bool nirIsStrictPredicate(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_GT:
    case NIR_FCMP_LT:
    case NIR_ICMP_UGT:
    case NIR_ICMP_ULT:
    case NIR_ICMP_SGT:
    case NIR_ICMP_SLT:
        return true;
    default:
        return false;
    }
}

bool nirIsEquality(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_ICMP_EQ:
    case NIR_ICMP_NE:
    case NIR_FCMP_EQ:
    case NIR_FCMP_NE:
        return true;
    default:
        return false;
    }
}

bool nirIsRelational(NIR_CMP_PREDICATE predicate)
{
    return !nirIsEquality(predicate);
}

bool nirIsSigned(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_ICMP_SGT:
    case NIR_ICMP_SGE:
    case NIR_ICMP_SLT:
    case NIR_ICMP_SLE:
        return true;
    default:
        return false;
    }
}

bool nirIsUnsigned(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_ICMP_UGT:
    case NIR_ICMP_UGE:
    case NIR_ICMP_ULT:
    case NIR_ICMP_ULE:
        return true;
    default:
        return false;
    }
}

bool nirIsTrueWhenEqual(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_EQ:
    case NIR_FCMP_GE:
    case NIR_FCMP_LE:

    case NIR_ICMP_EQ:

    case NIR_ICMP_UGE:
    case NIR_ICMP_ULE:

    case NIR_ICMP_SGE:
    case NIR_ICMP_SLE:
        return true;
    default:
        return false;
    }
}

bool nirIsFalseWhenEqual(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_NE:
    case NIR_FCMP_GT:
    case NIR_FCMP_LT:

    case NIR_ICMP_NE:

    case NIR_ICMP_UGT:
    case NIR_ICMP_ULT:

    case NIR_ICMP_SGT:
    case NIR_ICMP_SLT:
        return true;
    default:
        return false;
    }
}

bool nirIsImpliedTrueByMatchingCmp(NIR_CMP_PREDICATE predicate1,
                                   NIR_CMP_PREDICATE predicate2)
{
    if (predicate1 == predicate2)
        return true;

    switch (predicate1)
    {
    case NIR_ICMP_EQ:
        switch (predicate2)
        {
        case NIR_ICMP_UGE:
        case NIR_ICMP_ULE:
        case NIR_ICMP_SGE:
        case NIR_ICMP_SLE:
            // A == B implies any non-strict A (>=u, <=u, >=s, <=s) B are true.
            return true;
        default:
            return false;
        }
    case NIR_ICMP_UGT:
        // A >u B implies A !=u B and A >=u B are true.
        return predicate2 == NIR_ICMP_NE || predicate2 == NIR_ICMP_UGE;
    case NIR_ICMP_ULT:
        // A <u B implies A !=u B and A <=u B are true.
        return predicate2 == NIR_ICMP_NE || predicate2 == NIR_ICMP_ULE;
    case NIR_ICMP_SGT:
        // A >s B implies A !=s B and A >=s B are true.
        return predicate2 == NIR_ICMP_NE || predicate2 == NIR_ICMP_SGE;
    case NIR_ICMP_SLT:
        // A <u B implies A !=u B and A <=u B are true.
        return predicate2 == NIR_ICMP_NE || predicate2 == NIR_ICMP_SLE;
    default:
        return false;
    }
}

bool nirIsImpliedFalseByMatchingCmp(NIR_CMP_PREDICATE predicate1,
                                    NIR_CMP_PREDICATE predicate2)
{
    return nirIsImpliedTrueByMatchingCmp(predicate1,
                                         nirInversePredicate(predicate2));
}

const char *nirGetPredicateName(NIR_CMP_PREDICATE predicate)
{
    switch (predicate)
    {
    case NIR_FCMP_EQ:
        return "eq";
    case NIR_FCMP_GT:
        return "gt";
    case NIR_FCMP_GE:
        return "ge";
    case NIR_FCMP_LT:
        return "lt";
    case NIR_FCMP_LE:
        return "le";
    case NIR_FCMP_NE:
        return "ne";
    case NIR_FCMP_OR:
        return "or";
    case NIR_FCMP_UO:
        return "uo";

    case NIR_ICMP_EQ:
        return "eq";
    case NIR_ICMP_NE:
        return "ne";

    case NIR_ICMP_UGT:
        return "ugt";
    case NIR_ICMP_UGE:
        return "uge";
    case NIR_ICMP_ULT:
        return "ult";
    case NIR_ICMP_ULE:
        return "ule";

    case NIR_ICMP_SGT:
        return "sgt";
    case NIR_ICMP_SGE:
        return "sge";
    case NIR_ICMP_SLT:
        return "slt";
    case NIR_ICMP_SLE:
        return "sle";
    default:
        return "unknown";
    }
}

NIR_CMP_PREDICATE nirGetPredicate(NIR_VALUE *_cmp)
{
    TO_CMP(_cmp);
    return cmp->predicate;
}

NIR_CMP_PREDICATE nirGetInversePredicate(NIR_VALUE *_cmp)
{
    TO_CMP(_cmp);
    return nirInversePredicate(cmp->predicate);
}

NIR_VALUE *nirCreateCmp(NIR_CMP_PREDICATE predicate, NIR_VALUE *source1,
                        NIR_VALUE *source2, const char_t *name,
                        NIR_BASIC_BLOCK *block)
{
    assert(name);
    assert(block);

    NIR_CONTEXT *const context = nirGetBlockContext(block);

    NIR_CMP_INST *cmp = (NIR_CMP_INST *)ntMalloc(sizeof(NIR_CMP_INST));

    // value
    cmp->name = nirGetPrefixedId(context, name);
    cmp->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    cmp->dbgLoc = NULL;
    cmp->type = nirGetInt1Type(context);
    cmp->parent = NULL;

    // instruction
    cmp->opcode = NIR_OTHER_CMP;

    // cmp
    cmp->source1 = source1;
    cmp->source2 = source2;
    cmp->predicate = predicate;

    insertInst(block, (NIR_VALUE *)cmp);
    return (NIR_VALUE *)cmp;
}

/************/
/* PHI Node */
/************/

NIR_VALUE *nirCreatePhi(NIR_TYPE *type, const char_t *name,
                        NIR_BASIC_BLOCK *block)
{
    assert(type);
    assert(name);
    assert(block);

    NIR_CONTEXT *const context = nirGetBlockContext(block);

    NIR_PHI_NODE *phi = (NIR_PHI_NODE *)ntMalloc(sizeof(NIR_PHI_NODE));

    // value
    phi->name = nirGetPrefixedId(context, name);
    phi->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    phi->dbgLoc = NULL;
    phi->type = type;
    phi->parent = NULL;

    // instruction
    phi->opcode = NIR_OTHER_PHI;

    // phi
    listInit(&phi->list);

    insertInst(block, (NIR_VALUE *)phi);
    return (NIR_VALUE *)phi;
}

size_t nirGetNumIncomingValues(NIR_VALUE *_phi)
{
    TO_PHI(_phi);
    return phi->list.count;
}

NIR_VALUE *nirGetIncomingValue(NIR_VALUE *_phi, size_t i)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    return phi->incomingValues[i]->value;
}

void nirSetIncomingValue(NIR_VALUE *_phi, size_t i, NIR_VALUE *value)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    phi->incomingValues[i]->value = value;
}

NIR_BASIC_BLOCK *nirGetIncomingBlock(NIR_VALUE *_phi, size_t i)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    return phi->incomingValues[i]->block;
}

void nirSetIncomingBlock(NIR_VALUE *_phi, size_t i, NIR_BASIC_BLOCK *block)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    phi->incomingValues[i]->block = block;
}

void nirAddIncoming(NIR_VALUE *_phi, NIR_VALUE *value, NIR_BASIC_BLOCK *block)
{
    assert(value);
    assert(block);

    TO_PHI(_phi);

    NIR_INCOMING_VALUE *iv =
        (NIR_INCOMING_VALUE *)ntMalloc(sizeof(NIR_INCOMING_VALUE));
    iv->value = value;
    iv->block = block;

    listAdd(&phi->list, iv);
}

NIR_VALUE *nirRemoveIncomingValue(NIR_VALUE *_phi, size_t i)
{
    TO_PHI(_phi);
    assert(i <= phi->list.count);
    NIR_VALUE *value = phi->incomingValues[i]->value;
    listRemove(&phi->list, i);
    return value;
}

NIR_VALUE *nirRemoveIncomingBlock(NIR_VALUE *phi, NIR_BASIC_BLOCK *block)
{
    const size_t index = nirGetPhiBasicBlockIndex(phi, block);
    return nirRemoveIncomingValue(phi, index);
}

size_t nirGetPhiBasicBlockIndex(NIR_VALUE *_phi, NIR_BASIC_BLOCK *block)
{
    TO_PHI(_phi);
    assert(block);
    for (size_t i = 0; i < phi->list.count; ++i)
    {
        NIR_INCOMING_VALUE *iv = phi->incomingValues[i];
        if (iv->block == block)
            return i;
    }
    assert(0);
    return -1;
}

NIR_VALUE *nirGetIncomingValueForBlock(NIR_VALUE *_phi, NIR_BASIC_BLOCK *block)
{
    TO_PHI(_phi);
    assert(block);
    for (size_t i = 0; i < phi->list.count; ++i)
    {
        NIR_INCOMING_VALUE *iv = phi->incomingValues[i];
        if (iv->block == block)
            return iv->value;
    }
    return NULL;
}

NIR_VALUE *nirPhiHasConstantValue(NIR_VALUE *_phi)
{
    TO_PHI(_phi);
    assert(phi->list.count > 0);

    NIR_VALUE *constant = phi->incomingValues[0]->value;
    for (size_t i = 1, count = phi->list.count; i != count; ++i)
    {
        NIR_VALUE *const v = phi->incomingValues[i]->value;
        if (v == constant || v == _phi)
            continue;
        if (constant != _phi)
            return NULL;
        constant = v;
    }
    if (constant == _phi)
        return NIR_VALUE_UNDEF;
    return constant;
}

bool nirPhiHasConstantOrUndefValue(NIR_VALUE *_phi)
{
    TO_PHI(_phi);
    assert(phi->list.count > 0);

    NIR_VALUE *constant = NULL;
    for (size_t i = 0, count = phi->list.count; i != count; ++i)
    {
        NIR_VALUE *const v = phi->incomingValues[i]->value;
        if (v == _phi || v == NIR_VALUE_UNDEF)
            continue;
        if (constant && constant != v)
            return false;
        constant = v;
    }
    return true;
}

bool nirPhiIsComplete(NIR_VALUE *_phi)
{
    TO_PHI(_phi);

    NIR_BASIC_BLOCK *parent = phi->parent;
    assert(parent);

    const size_t count = nirGetPredecessorCount(parent);
    const size_t incomingCount = phi->list.count;

    if (count < incomingCount)
        return false; // missing incoming values

    for (size_t i = 0; i < count; ++i)
    {
        NIR_BASIC_BLOCK *const predecessor = nirGetPredecessor(parent, i);

        bool find = false;
        for (size_t j = 0; j < incomingCount; ++j)
        {
            NIR_BASIC_BLOCK *incomingBlock = phi->incomingValues[j]->block;
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

NIR_VALUE *nirCreateReturn(NIR_VALUE *returnValue, NIR_BASIC_BLOCK *block)
{
    assert(block);

    NIR_RETURN_INST *ret = (NIR_RETURN_INST *)ntMalloc(sizeof(NIR_RETURN_INST));

    NIR_CONTEXT *context = nirGetBlockContext(block);

    // value
    ret->name = NULL;
    ret->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    ret->dbgLoc = NULL;
    ret->type = nirGetVoidType(context);
    ret->parent = NULL;

    // instruction
    ret->opcode = NIR_TERM_RET;

    // ret
    ret->retValue = returnValue;

    insertInst(block, (NIR_VALUE *)ret);
    return (NIR_VALUE *)ret;
}

NIR_VALUE *nirGetReturnValue(NIR_VALUE *_ret)
{
    TO_RET(_ret);
    return ret->retValue;
}

/**********/
/* Select */
/**********/

NIR_VALUE *nirCreateSelect(NIR_VALUE *condition, NIR_VALUE *valueWhenTrue,
                           NIR_VALUE *valueWhenFalse, const char_t *name,
                           NIR_BASIC_BLOCK *block)
{
    assert(block);
    assert(condition);
    assert(valueWhenTrue);
    assert(valueWhenFalse);
    assert(name);

    NIR_CONTEXT *const context = nirGetBlockContext(block);

    assert(nirGetInt1Type(context) == nirGetType(condition));
    assert(nirGetVoidType(context) != nirGetType(valueWhenTrue));
    assert(nirGetVoidType(context) != nirGetType(valueWhenFalse));
    assert(nirGetType(valueWhenTrue) == nirGetType(valueWhenFalse));

    NIR_SELECT_INST *sel = (NIR_SELECT_INST *)ntMalloc(sizeof(NIR_SELECT_INST));

    // value
    sel->name = nirGetPrefixedId(context, name);
    sel->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    sel->dbgLoc = NULL;
    sel->type = nirGetType(valueWhenTrue);
    sel->parent = NULL;

    // instruction
    sel->opcode = NIR_OTHER_SELECT;

    // sel
    sel->condition = condition;
    sel->trueValue = valueWhenTrue;
    sel->falseValue = valueWhenFalse;

    insertInst(block, (NIR_VALUE *)sel);
    return (NIR_VALUE *)sel;
}

// see nirGetCondition in branch
// NIR_VALUE *nirGetCondition(NIR_VALUE *select)

NIR_VALUE *nirGetTrueValue(NIR_VALUE *select)
{
    TO_SEL(select);
    return sel->trueValue;
}

NIR_VALUE *nirGetFalseValue(NIR_VALUE *select)
{
    TO_SEL(select);
    return sel->falseValue;
}

// see nirSetCondition in branch
// void nirSetCondition(NIR_VALUE *select, NIR_VALUE *condition)

void nirSetTrueValue(NIR_VALUE *select, NIR_VALUE *value)
{
    TO_SEL(select);
    sel->trueValue = value;
}

void nirSetFalseValue(NIR_VALUE *select, NIR_VALUE *value)
{
    TO_SEL(select);
    sel->falseValue = value;
}

void nirSwapValues(NIR_VALUE *select)
{
    TO_SEL(select);
    NIR_VALUE *const tmp = sel->trueValue;
    sel->trueValue = sel->falseValue;
    sel->falseValue = tmp;
}

const char *nirAreSelectInvalidOperands(NIR_VALUE *condition,
                                        NIR_VALUE *trueValue,
                                        NIR_VALUE *falseValue)
{
    assert(condition);
    assert(trueValue);
    assert(falseValue);

    if (nirGetType(trueValue) == nirGetType(falseValue))
        return "both values must have same type";

    NIR_TYPE *conditionType = nirGetType(condition);
    NIR_CONTEXT *const context = nirGetTypeContext(conditionType);
    if (conditionType != nirGetInt1Type(context))
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
 * @return NIR_VALUE* that represents the instruction.
 */
NIR_VALUE *nirCreateStore(NIR_VALUE *value, NIR_VALUE *ptr,
                          NIR_BASIC_BLOCK *block)
{
    assert(block);
    assert(value);
    assert(ptr);

    NIR_CONTEXT *context = nirGetBlockContext(block);

    NIR_TYPE *ptrType = nirGetType(ptr);
    assert(nirIsPointerType(ptrType));
    // assert(nirGetPointeeType(ptrType) == nirGetType(value));

    NIR_STORE_INST *str = (NIR_STORE_INST *)ntMalloc(sizeof(NIR_STORE_INST));
    // value
    str->name = NULL;
    str->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    str->dbgLoc = NULL;
    str->type = nirGetVoidType(context);
    str->parent = NULL;

    // instruction
    str->opcode = NIR_OTHER_MEMORY_STORE;

    // sel
    str->source = value;
    str->ptr = ptr;

    insertInst(block, (NIR_VALUE *)str);
    return (NIR_VALUE *)str;
}

NIR_VALUE *nirGetPointerOperand(NIR_VALUE *store)
{
    TO_STR(store);
    return str->ptr;
}

NIR_TYPE *nirGetPointerOperandType(NIR_VALUE *store)
{
    TO_STR(store);
    return nirGetType(str->ptr);
}

NIR_VALUE *nirGetValueOperand(NIR_VALUE *store)
{
    TO_STR(store);
    return str->source;
}

/**********/
/* Unary */
/**********/

NIR_VALUE *nirCreateUnaryInst(NIR_OPCODE unaryOp, NIR_TYPE *valueType,
                              NIR_VALUE *value, const char_t *name,
                              NIR_BASIC_BLOCK *block)
{
    assert(nirIsUnaryOp(unaryOp) && "operation is not unary!");
    assert(name);
    assert(block);
    assert(valueType);

    NIR_CONTEXT *context = nirGetBlockContext(block);

    NIR_UNARY_OPERATOR *una =
        (NIR_UNARY_OPERATOR *)ntMalloc(sizeof(NIR_UNARY_OPERATOR));
    // value
    una->name = nirGetPrefixedId(context, name);
    una->valueType = NIR_VALUE_TYPE_INSTRUCTION;
    una->dbgLoc = NULL;
    una->type = valueType;
    una->parent = NULL;

    // instruction
    una->opcode = unaryOp;

    // sel
    una->source = value;

    insertInst(block, (NIR_VALUE *)una);
    return (NIR_VALUE *)una;
}

NIR_VALUE *nirGetUnaryValueOperand(NIR_VALUE *unary)
{
    TO_UNA(unary);
    return una->source;
}

NIR_TYPE *nirGetUnaryTypeOperand(NIR_VALUE *unary)
{
    TO_UNA(unary);
    return nirGetType(una->source);
}
