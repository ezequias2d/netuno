#ifndef NIR_INSTRUCTION_H
#define NIR_INSTRUCTION_H

#include <netuno/common.h>

NT_HANDLE(NIR_VALUE)
NT_HANDLE(NIR_TYPE)
NT_HANDLE(NIR_FUNCTION)
NT_HANDLE(NIR_BASIC_BLOCK)
NT_HANDLE(NIR_CONTEXT)

/**
 * @ingroup Instruction
 * @brief This enum indetifier type of instruction.
 * @typedef NIR_INST_TYPE
 * @see @ref NIR_INSTRUCTION
 */
NT_ENUM(NIR_INST_TYPE){
    NIR_INST_TYPE_BINARY_OPERATOR,
    NIR_INST_TYPE_UNARY_OPERATOR,
    NIR_INST_TYPE_BRANCH,
    NIR_INST_TYPE_CALL,
    NIR_INST_TYPE_CMP,
    NIR_INST_TYPE_PHI_NODE,
    NIR_INST_TYPE_RETURN,
    NIR_INST_TYPE_SELECT,
    NIR_INST_TYPE_STORE,
};

NT_ENUM(NIR_OPCODE){
    // terminator instructions
    NIR_TERM_RET,
    NIR_TERM_OPS_BEGIN = NIR_TERM_RET,
    NIR_TERM_BR,
    // NIR_TERM_INDIRECT_BR,

    // unary operators
    NIR_UNARY_FNEG,
    NIR_TERM_OPS_END = NIR_UNARY_FNEG,
    NIR_UNARY_OPS_BEGIN = NIR_UNARY_FNEG,
    NIR_UNARY_MEMORY_ALLOCA,
    NIR_UNARY_MEMORY_LOAD,
    NIR_UNARY_CAST_TRUNC,
    NIR_UNARY_CAST_ZEXT,
    NIR_UNARY_CAST_SEXT,
    NIR_UNARY_CAST_FP_TO_UI,
    NIR_UNARY_CAST_FP_TO_SI,
    NIR_UNARY_CAST_UI_TO_FP,
    NIR_UNARY_CAST_SI_TO_FP,
    NIR_UNARY_CAST_FP_TRUNC,
    NIR_UNARY_CAST_PTR_TO_INT,
    NIR_UNARY_CAST_INT_TO_PTR,
    NIR_UNARY_CAST_BIT_CAST,

    // binary operators
    NIR_BINARY_OP_ADD,
    NIR_UNARY_OPS_END = NIR_BINARY_OP_ADD,
    NIR_BINARY_OPS_BEGIN = NIR_BINARY_OP_ADD,
    NIR_BINARY_OP_FADD,
    NIR_BINARY_OP_SUB,
    NIR_BINARY_OP_FSUB,
    NIR_BINARY_OP_MUL,
    NIR_BINARY_OP_FMUL,
    NIR_BINARY_OP_UDIV,
    NIR_BINARY_OP_SDIV,
    NIR_BINARY_OP_FDIV,
    NIR_BINARY_OP_UREM,
    NIR_BINARY_OP_SREM,
    NIR_BINARY_OP_FREM,

    // logical operators
    NIR_BINARY_OP_SHL, // Shift left  (logical)
    NIR_BINARY_OP_SHR, // Shift right (logical)
    NIR_BINARY_OP_ASR, // Shift right (arithmetic)
    NIR_BINARY_OP_AND,
    NIR_BINARY_OP_OR,
    NIR_BINARY_OP_XOR,

    // others
    NIR_GET_ELEMENT_PTR,
    NIR_BINARY_OPS_END = NIR_GET_ELEMENT_PTR,
    NIR_OTHER_OPS_BEGIN = NIR_GET_ELEMENT_PTR,
    NIR_EXTRACT_VALUE,
    NIR_OTHER_CMP,
    NIR_OTHER_PHI,
    NIR_OTHER_CALL,
    NIR_OTHER_SELECT,
    NIR_OTHER_MEMORY_STORE,

    NIR_OTHER_INSERT_VALUE,
    NIR_OTHER_LAST,
};

/**
 * @brief Gets opcode name.
 *
 * @param opcode Instruction.
 * @return const char* Opcode name of instruction.
 */
const char *nirGetOpcodeName(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is a terminator instruction.
 *
 * @param opcode Instruction.
 * @return true if opcode is terminator
 * @return false if opcode is terminator
 */
bool nirIsTermiantor(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of unary instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is unary.
 * @return false if opcode is not unary.
 */
bool nirIsUnaryOp(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of binary instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is binary.
 * @return false if opcode is not binary.
 */
bool nirIsBinaryOp(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the div or rem instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a div or rem instruction.
 * @return false if opcode is not a div and rem instruction.
 */
bool nirIsIntDivRem(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the shift instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a shift instruction.
 * @return false if opcode is not a shift instruction.
 */
bool nirIsShift(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the logical shift instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a logical shift instruction.
 * @return false if opcode is not a logical shift instruction.
 */
bool nirIsLogicalShift(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the artihmetic shift instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a arithmetic shift instruction.
 * @return false if opcode is not a arithmetic shift instruction.
 */
bool nirIsArithmeticShift(NIR_OPCODE opcode);

/**
 * @brief Determine if the opcode is and/or/xor.
 *
 * @param opcode Instruction.
 * @return true if opcode is a and/or/xor.
 * @return false if opcode is not a and/or/xor.
 */
bool nirIsBitwiseLogicOp(NIR_OPCODE opcode);

/**
 * @brief Determine if the Opcode is one of the cast instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a cast.
 * @return false if opcode is not a cast.
 */
bool nirIsCast(NIR_OPCODE opcode);

/**
 * @brief Determine if the instruction is associative.
 *
 * Associative operators satisfy: A op (B op C) === (A op B) op C
 *
 * Add, Mul, And, Or and Xor are associative.
 *
 * @param opcode Instruction.
 * @return true if instruction is associative.
 * @return false if instruction is not associative.
 */
bool nirIsAssociative(NIR_OPCODE opcode);

/**
 * @brief Determine if the instruction is commutative.
 *
 * Commutative operators satisfy: (A op B) === (B op A)
 *
 * Associative operators plus SetEQ and SetNE are commutative.
 *
 * @param opcode Instruction.
 * @return true if instruction is commutative.
 * @return false if instruction is not commutative.
 */
bool nirIsCommutative(NIR_OPCODE opcode);

/**
 * @brief Determine if the instruction is idempotent.
 *
 * Idempotent operators satisfy: (A op A) === A
 *
 * @param opcode Instruction.
 * @return true if instruction is idempotent.
 * @return false if instruction is idempotent.
 */
bool nirIsIdempotent(NIR_OPCODE opcode);

/**
 * @brief Determine if the instruction is nilpotent.
 *
 * Nilpotent operators satisfy: (A op A) === identity
 *
 * identity is a constant such that
 *  (A op identity) === A and (identity op A) === A for all A.
 *
 * @param opcode Instruction.
 * @return true if instruction is nilpotent.
 * @return false if instruction is nilpotent.
 */
bool nirIsNilpotent(NIR_OPCODE opcode);

NIR_OPCODE nirGetOpcode(NIR_VALUE *instruction);

/**********/
/* Binary */
/**********/

/**
 * @brief Construct a binary instruction, given the opcode and the two operands.
 * Also automatically insert this instruction to the end of the BasicBlock
 * specified.
 *
 * @param op Opcode.
 * @param source1 First operand.
 * @param source2 Second operand.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the output of this instruction.
 */
NIR_VALUE *nirCreateBinary(NIR_OPCODE op, NIR_VALUE *source1,
                           NIR_VALUE *source2, const char_t *name,
                           NIR_BASIC_BLOCK *block);

/**
 * @brief Helper for construct the unary NEG operation via binary operator SUB.
 *
 * @param source First operand.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the output of this instruction.
 */
NIR_VALUE *nirCreateNeg(NIR_VALUE *source, const char_t *name,
                        NIR_BASIC_BLOCK *block);

/**
 * @brief Helper for construct the unary NOT operation via binary operator XOR.
 *
 * @param source First operand.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the output of this instruction.
 */
NIR_VALUE *nirCreateNot(NIR_VALUE *source, const char_t *name,
                        NIR_BASIC_BLOCK *block);

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
                            NIR_BASIC_BLOCK *block);

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
                            NIR_BASIC_BLOCK *block);

/**
 * @brief Determine if the instruction is uncoditional.
 *
 * @param branch Branch instruction.
 * @return true if is uncoditional branch instruction.
 * @return false if is not uncoditional branch instruction.
 */
bool nirIsUnconditional(NIR_VALUE *branch);

/**
 * @brief Determine if the instruction is coditional.
 *
 * @param branch Branch instruction.
 * @return true if is coditional branch instruction.
 * @return false if is not coditional branch instruction.
 */
bool nirIsConditional(NIR_VALUE *branch);

/**
 * @brief Gets condition value.
 *
 * @param branch Branch instruction.
 * @return NIR_VALUE* condition value of branch instruction, otherwise NULL.
 */
NIR_VALUE *nirGetCondition(NIR_VALUE *branch);

/**
 * @brief Sets condition value.
 *
 * @param branch Branch instruction.
 * @param condition condition value to use in branch.
 */
void nirSetCondition(NIR_VALUE *branch, NIR_VALUE *condition);

/**
 * @brief Gets number of sucessors BasicBlocks.
 *
 * @param term Terminator instruction.
 * @return size_t number of sucessors BasicBlocks.
 */
size_t nirGetSucessorCount(NIR_VALUE *term);

/**
 * @brief Sets a BasicBlock sucessor in a index.
 *
 * @param index Sucessor index, must be bounded by number of sucessors.
 * @param newBasicBlockSucessor New BasicBlock sucessor.
 */
void nirSetSuccessor(NIR_VALUE *branch, size_t index,
                     NIR_BASIC_BLOCK *newBasicBlockSucessor);

/********/
/* Call */
/********/

NIR_VALUE *nirCreateCall(NIR_TYPE *functionType, NIR_FUNCTION *function,
                         size_t argCount, NIR_VALUE **args, const char_t *name,
                         NIR_BASIC_BLOCK *block);

NIR_TYPE *nirGetCallFunctionType(NIR_VALUE *call);
NIR_FUNCTION *nirGetCaller(NIR_VALUE *call);
void nirSetCaller(NIR_VALUE *call, NIR_TYPE *functionType,
                  NIR_FUNCTION *function);
size_t nirGetArgSize(NIR_VALUE *call);
NIR_VALUE *nirGetArgOperand(NIR_VALUE *call, size_t index);
void nirSetArgOperand(NIR_VALUE *call, size_t index, NIR_VALUE *value);

/****************/
/* Compare(CMP) */
/****************/

NT_ENUM(NIR_CMP_PREDICATE){
    /// @brief True if equal
    NIR_FCMP_EQ,
    /// @brief True if greater than
    NIR_FCMP_GT,
    /// @brief True if greater than or equal
    NIR_FCMP_GE,
    /// @brief True if less than
    NIR_FCMP_LT,
    /// @brief True if less than or equal
    NIR_FCMP_LE,
    /// @brief True if operands are unequal
    NIR_FCMP_NE,
    /// @brief True if ordered: (no nan)
    NIR_FCMP_OR,
    /// @brief True if unordered: isnan(x) | isnan(y)
    NIR_FCMP_UO,

    /// @brief True if equal
    NIR_ICMP_EQ,
    /// @brief True if unequal
    NIR_ICMP_NE,

    /// @brief True if unsigned greater than
    NIR_ICMP_UGT,
    /// @brief True if unsigned greater than or equal
    NIR_ICMP_UGE,
    /// @brief True if unsigned less than
    NIR_ICMP_ULT,
    /// @brief True if unsigned less than or equal
    NIR_ICMP_ULE,

    /// @brief True if signed greater than
    NIR_ICMP_SGT,
    /// @brief True if signed greater than or equal
    NIR_ICMP_SGE,
    /// @brief True if signed less than
    NIR_ICMP_SLT,
    /// @brief True if signed less than or equal
    NIR_ICMP_SLE,

    NIR_ICMP_FIRST = NIR_ICMP_EQ,
    NIR_ICMP_LAST = NIR_ICMP_SLE,
    NIR_FCMP_FIRST = NIR_FCMP_EQ,
    NIR_FCMP_LAST = NIR_FCMP_UO,
};

/**
 * @brief Retrieves inverse predicate.
 * For example EQ -> NE, GT -> LE, LT -> GE, OR -> UO, SGT -> SLE, SLT -> SGE,
 * etc.
 *
 * @param predicate Predicate.
 * @return NIR_CMP_PREDICATE opositive predicate.
 */
NIR_CMP_PREDICATE nirInversePredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Retrieves strict version of a non-strict comparison.
 * For example GE -> GT, LE -> LT, UGE -> UGT, ULE -> ULT, SGE -> SGT, SLE ->
 * SLT.
 *
 * @param predicate Predicate.
 * @return NIR_CMP_PREDICATE Strict version.
 */
NIR_CMP_PREDICATE nirStrictPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Retrieves non-strict version of a strict comparison.
 * For example GT -> GE, LT -> LE, UGT -> UGE, ULT -> ULE, SGT -> SGE, SLT ->
 * SLE.
 *
 * @param predicate Predicate.
 * @return NIR_CMP_PREDICATE Non-strict version.
 */
NIR_CMP_PREDICATE nirNonStrictPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Retrieves signed version of a predicate.
 * For example ULT -> SLT, ULE -> SLE, UGT -> SGT, UGE -> SGE, SLT -> failed
 * assert.
 *
 * @param predicate Predicate.
 * @return NIR_CMP_PREDICATE Signed version.
 */
NIR_CMP_PREDICATE nirSignedPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Retrieves unsigned version of a predicate.
 * For example SLT -> ULT, SLE -> ULE, SGT -> UGT, SGE -> UGE, ULT -> failed
 * assert.
 *
 * @param predicate Predicate.
 * @return NIR_CMP_PREDICATE Unsigned version.
 */
NIR_CMP_PREDICATE nirUnsignedPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is a INT predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is a INT predicate.
 * @return false if predicate is not a INT predicate.
 */
bool nirIsIntPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is a FLOAT predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is a FLOAT predicate.
 * @return false if predicate is a FLOAT predicate.
 */
bool nirIsFPPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is strict predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is strict (false when operands are equal).
 * @return false if predicate is not strict.
 */
bool nirIsStrictPredicate(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is non-strict predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is non-strict (true when operands are equal).
 * @return false if predicate is not non-strict;
 */
bool nirIsEquality(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is relational.
 *
 * @param predicate Predicate.
 * @return true if predicate is relational (not EQ or NE)
 * @return false if predicate is not a relational
 */
bool nirIsRelational(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is signed.
 *
 * @param predicate Predicate.
 * @return true if predicate is signed.
 * @return false if predicate is not a signed.
 */
bool nirIsSigned(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is unsigned.
 *
 * @param predicate Predicate.
 * @return true if predicate is unsigned.
 * @return false if predicate is not a signed.
 */
bool nirIsUnsigned(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if the predicate is true when comparing a value with itself.
 *
 * @param predicate Predicate.
 * @return true if predicate is true when comparing a value with itself.
 * @return false if predicate is not true when comparing a value with itself.
 */
bool nirIsTrueWhenEqual(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if the predicate is false when comparing a value with
 * itself.
 *
 * @param predicate Predicate.
 * @return true if predicate is false when comparing a value with itself.
 * @return false if predicate is not false when comparing a value with itself.
 */
bool nirIsFalseWhenEqual(NIR_CMP_PREDICATE predicate);

/**
 * @brief Determine if the first condition implies the second is true.
 *
 * @param predicate1 First condition predicate.
 * @param predicate2 Second condition predicate.
 * @return true if first condition implies the second condition is true.
 * @return false if first condition not implies the second condition is true.
 */
bool nirIsImpliedTrueByMatchingCmp(NIR_CMP_PREDICATE predicate1,
                                   NIR_CMP_PREDICATE predicate2);

/**
 * @brief Determine if the first condition implies the second is false.
 *
 * @param predicate1 First codition predicate.
 * @param predicate2 Second condition predicate.
 * @return true if first condition implies the second condition is false.
 * @return false if first condition not implies the second condition is false.
 */
bool nirIsImpliedFalseByMatchingCmp(NIR_CMP_PREDICATE predicate1,
                                    NIR_CMP_PREDICATE predicate2);

/**
 * @brief Gets predicate name;
 *
 * @param predicate Predicate.
 * @return const char* predicate name.
 */
const char *nirGetPredicateName(NIR_CMP_PREDICATE predicate);

/**
 * @brief Gets predicate from a CMP instruction.
 *
 * @param cmp CMP instruction.
 * @return NIR_CMP_PREDICATE predicate of CMP instruction.
 */
NIR_CMP_PREDICATE nirGetPredicate(NIR_VALUE *cmp);

/**
 * @brief Gets inverse predicate from a CMP instruction.
 *
 * @param cmp CMP instruction.
 * @return NIR_CMP_PREDICATE inverse predicate of CMP instruction.
 */
NIR_CMP_PREDICATE nirGetInversePredicate(NIR_VALUE *cmp);

/**
 * @brief Construct a CMP node instruction, given a predicate and two values.
 * Also automatically insert this instruction to the end of the BasicBlock
 * specified.
 *
 * @param predicate Compare predicate.
 * @param source1 First(and left) source value.
 * @param source2 Second(and right) source value.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the instruction.
 */
NIR_VALUE *nirCreateCmp(NIR_CMP_PREDICATE predicate, NIR_VALUE *source1,
                        NIR_VALUE *source2, const char_t *name,
                        NIR_BASIC_BLOCK *block);

/************/
/* PHI Node */
/************/

/**
 * @brief Construct a PHI node instruction, given a value type. Also
 * automatically insert this instruction to the end of the BasicBlock specified.
 *
 * @param valueType PHI node value type.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the instruction.
 */
NIR_VALUE *nirCreatePhi(NIR_TYPE *valueType, const char_t *name,
                        NIR_BASIC_BLOCK *block);

/**
 * @brief Retrieves the number of incoming values.
 *
 * @param phi PHI node.
 * @return size_t number of incoming values.
 */
size_t nirGetNumIncomingValues(NIR_VALUE *phi);

/**
 * @brief Retrieves the incoming value for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @return NIR_VALUE* incoming value.
 */
NIR_VALUE *nirGetIncomingValue(NIR_VALUE *phi, size_t i);

/**
 * @brief Defines the incoming value for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @param value Incoming value.
 */
void nirSetIncomingValue(NIR_VALUE *phi, size_t i, NIR_VALUE *value);

/**
 * @brief Retrieves the incoming block for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @return NIR_VALUE* incoming block of the index.
 */
NIR_BASIC_BLOCK *nirGetIncomingBlock(NIR_VALUE *phi, size_t i);

/**
 * @brief Defines the incoming block for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @param basicBlock Incoming block.
 */
void nirSetIncomingBlock(NIR_VALUE *phi, size_t i, NIR_BASIC_BLOCK *block);

/**
 * @brief Add an incoming value to the end of the PHI node.
 *
 * @param phi PHI node.
 * @param value Incoming value.
 * @param basicBlock Incoming block.
 */
void nirAddIncoming(NIR_VALUE *phi, NIR_VALUE *value, NIR_BASIC_BLOCK *block);

/**
 * @brief Remove an incoming value by index. Also automatically
 * destroy this instruction if are empty and replace all usage of this
 * instruction with undefined values.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @return NIR_VALUE* the removed incoming value.
 */
NIR_VALUE *nirRemoveIncomingValue(NIR_VALUE *phi, size_t i);

/**
 * @brief Remove an incoming value by incoming block. Also automatically
 * destroy this instruction if are empty and replace all usage of this
 * instruction with undefined values.
 *
 * @param phi PHI node.
 * @param basicBlock Incoming block.
 * @return NIR_VALUE* the removed incoming value.
 */
NIR_VALUE *nirRemoveIncomingBlock(NIR_VALUE *phi, NIR_BASIC_BLOCK *block);

/**
 * @brief Retrieves the incoming value index of a basic block.
 *
 * @param phi PHI node.
 * @param basicBlock Incoming block.
 * @return size_t incoming value index for a basic block.
 */
size_t nirGetPhiBasicBlockIndex(NIR_VALUE *phi, NIR_BASIC_BLOCK *block);

/**
 * @brief Retrieves the incoming value for a basic block.
 *
 * @param phi PHI node.
 * @param basicBlock Incoming block.
 * @return NIR_VALUE* incoming value for the basic block, otherwise NULL.
 */
NIR_VALUE *nirGetIncomingValueForBlock(NIR_VALUE *phi, NIR_BASIC_BLOCK *block);

/**
 * @brief Determine if the PHI node always merges together the same value.
 *
 * @param phi PHI node.
 * @return NIR_VALUE* the same value, otherwise NULL.
 */
NIR_VALUE *nirPhiHasConstantValue(NIR_VALUE *phi);

/**
 * @brief Determine if the PHI node always mergers together the same value,
 * assuming undefined values are the same value as non-undefined values.
 *
 * @param phi PHI node.
 * @return true If all defined values merges to same value.
 * @return false If any defined value do not merge to same value.
 */
bool nirPhiHasConstantOrUndefValue(NIR_VALUE *phi);

/**
 * @brief Determine if the PHI node is complete (all parent's predecessors have
 * incoming value).
 *
 * @param phi PHI node.
 * @return true if all parent's predecessors have incoming value in this PHI
 * node.
 * @return false if any parent's predecessors have no incoming value in this PHI
 * node.
 */
bool nirPhiIsComplete(NIR_VALUE *phi);

/**********/
/* Return */
/**********/

/**
 * @brief Construct a return instruction, given return value. Also automatically
 * insert this instruction to the end of the BasicBlock specified.
 *
 * @param context NIR_CONTEXT
 * @param returnValue The value that will be returned.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the instruction.
 */
NIR_VALUE *nirCreateReturn(NIR_VALUE *returnValue, NIR_BASIC_BLOCK *block);

/**
 * @brief Retrieves the return value of a return instruction.
 *
 * @param ret Return instruction.
 * @return NIR_VALUE* The return value of the return instruction, otherwise
 * NULL.
 */
NIR_VALUE *nirGetReturnValue(NIR_VALUE *ret);

/**********/
/* Select */
/**********/

/**
 * @brief Construct a select instruction, given true value, false value and
 * codition. Also automatically insert this instruction to the end of the
 * BasicBlock specified.
 *
 * @param condition Codition value, must be i1.
 * @param valueWhenTrue Value when codition are true.
 * @param valueWhenFalse Value when codition are false.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIR_VALUE* Value that represents the instruction.
 */
NIR_VALUE *nirCreateSelect(NIR_VALUE *condition, NIR_VALUE *valueWhenTrue,
                           NIR_VALUE *valueWhenFalse, const char_t *name,
                           NIR_BASIC_BLOCK *block);

/**
 * @brief Retrieves the condition value of a select instruction.
 *
 * @param select Select instruction.
 * @return NIR_VALUE* condition value.
 */
NIR_VALUE *nirGetCondition(NIR_VALUE *select);

/**
 * @brief Retrieves the true value of a select instruction.
 *
 * @param select Select instruction.
 * @return NIR_VALUE* true value.
 */
NIR_VALUE *nirGetTrueValue(NIR_VALUE *select);

/**
 * @brief Retrieves the false value of a select instruction.
 *
 * @param select Select instruction.
 * @return NIR_VALUE* false value.
 */
NIR_VALUE *nirGetFalseValue(NIR_VALUE *select);

/**
 * @brief Defines the condition value of a select instruction.
 *
 * @param select Select instruction.
 * @param value Condition value.
 */
void nirSetCondition(NIR_VALUE *select, NIR_VALUE *condition);

/**
 * @brief Defines the true value of a select instruction.
 *
 * @param select Select instruction.
 * @param value True value.
 */
void nirSetTrueValue(NIR_VALUE *select, NIR_VALUE *value);

/**
 * @brief Defines the false value of a select instruction.
 *
 * @param select Select instruction.
 * @param value False value.
 */
void nirSetFalseValue(NIR_VALUE *select, NIR_VALUE *value);

/**
 * @brief Swap the true and false values of the select instruction.
 *
 * @param select Select instruction.
 */
void nirSwapValues(NIR_VALUE *select);

/**
 * @brief Determine if the specified operands are invalid for a select
 * instruction.
 *
 * @param condition Condition value.
 * @param trueValue True value.
 * @param falseValue False value.
 * @return const char* a string if the specified operands are invalid for a
 * select instruction, otherwise NULL.
 */
const char *nirAreSelectInvalidOperands(NIR_VALUE *condition,
                                        NIR_VALUE *trueValue,
                                        NIR_VALUE *falseValue);

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
                          NIR_BASIC_BLOCK *block);

/**
 * @brief Retrieves the pointer operand of store instruction.
 *
 * @param store Store instruction.
 * @return NIR_VALUE* that is the pointer operand.
 */
NIR_VALUE *nirGetPointerOperand(NIR_VALUE *store);

/**
 * @brief Retrieves the pointer operand type of store instruction.
 *
 * @param store Store instruction.
 * @return NIR_TYPE* that is the type of pointer operand.
 */
NIR_TYPE *nirGetPointerOperandType(NIR_VALUE *store);

/**
 * @brief Retrieves the value operand of store instruction.
 *
 * @param store Store instruction.
 * @return NIR_VALUE* that is the value operand.
 */
NIR_VALUE *nirGetValueOperand(NIR_VALUE *store);

/**********/
/* Unary */
/**********/

NIR_VALUE *nirCreateUnaryInst(NIR_OPCODE unaryOp, NIR_TYPE *valueType,
                              NIR_VALUE *value, const char_t *name,
                              NIR_BASIC_BLOCK *block);

NIR_VALUE *nirGetUnaryValueOperand(NIR_VALUE *unary);
NIR_TYPE *nirGetUnaryTypeOperand(NIR_VALUE *unary);

#endif
