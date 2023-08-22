#ifndef NIL_INSTRUCTION_H
#define NIL_INSTRUCTION_H

#include <netuno/common.h>

NT_HANDLE(NIL_VALUE)
NT_HANDLE(NIL_TYPE)
NT_HANDLE(NIL_FUNCTION)
NT_HANDLE(NIL_BASIC_BLOCK)
NT_HANDLE(NIL_CONTEXT)

/**
 * @ingroup Instruction
 * @brief This enum indetifier type of instruction.
 * @typedef NIL_INST_TYPE
 * @see @ref NIL_INSTRUCTION
 */
NT_ENUM(NIL_INST_TYPE){
    NIL_INST_TYPE_BINARY_OPERATOR,
    NIL_INST_TYPE_UNARY_OPERATOR,
    NIL_INST_TYPE_BRANCH,
    NIL_INST_TYPE_CALL,
    NIL_INST_TYPE_CMP,
    NIL_INST_TYPE_PHI_NODE,
    NIL_INST_TYPE_RETURN,
    NIL_INST_TYPE_SELECT,
    NIL_INST_TYPE_STORE,
};

NT_ENUM(NIL_OPCODE){
    // terminator instructions
    NIL_TERM_RET,
    NIL_TERM_OPS_BEGIN = NIL_TERM_RET,
    NIL_TERM_BR,
    // NIL_TERM_INDIRECT_BR,

    // unary operators
    NIL_UNARY_FNEG,
    NIL_TERM_OPS_END = NIL_UNARY_FNEG,
    NIL_UNARY_OPS_BEGIN = NIL_UNARY_FNEG,
    NIL_UNARY_MEMORY_ALLOCA,
    NIL_UNARY_MEMORY_LOAD,
    NIL_UNARY_CAST_TRUNC,
    NIL_UNARY_CAST_ZEXT,
    NIL_UNARY_CAST_SEXT,
    NIL_UNARY_CAST_FP_TO_UI,
    NIL_UNARY_CAST_FP_TO_SI,
    NIL_UNARY_CAST_UI_TO_FP,
    NIL_UNARY_CAST_SI_TO_FP,
    NIL_UNARY_CAST_FP_TRUNC,
    NIL_UNARY_CAST_PTR_TO_INT,
    NIL_UNARY_CAST_INT_TO_PTR,
    NIL_UNARY_CAST_BIT_CAST,

    // binary operators
    NIL_BINARY_OP_ADD,
    NIL_UNARY_OPS_END = NIL_BINARY_OP_ADD,
    NIL_BINARY_OPS_BEGIN = NIL_BINARY_OP_ADD,
    NIL_BINARY_OP_FADD,
    NIL_BINARY_OP_SUB,
    NIL_BINARY_OP_FSUB,
    NIL_BINARY_OP_MUL,
    NIL_BINARY_OP_FMUL,
    NIL_BINARY_OP_UDIV,
    NIL_BINARY_OP_SDIV,
    NIL_BINARY_OP_FDIV,
    NIL_BINARY_OP_UREM,
    NIL_BINARY_OP_SREM,
    NIL_BINARY_OP_FREM,

    // logical operators
    NIL_BINARY_OP_SHL, // Shift left  (logical)
    NIL_BINARY_OP_SHR, // Shift right (logical)
    NIL_BINARY_OP_ASR, // Shift right (arithmetic)
    NIL_BINARY_OP_AND,
    NIL_BINARY_OP_OR,
    NIL_BINARY_OP_XOR,

    // others
    NIL_GET_ELEMENT_PTR,
    NIL_BINARY_OPS_END = NIL_GET_ELEMENT_PTR,
    NIL_OTHER_OPS_BEGIN = NIL_GET_ELEMENT_PTR,
    NIL_EXTRACT_VALUE,
    NIL_OTHER_CMP,
    NIL_OTHER_PHI,
    NIL_OTHER_CALL,
    NIL_OTHER_SELECT,
    NIL_OTHER_MEMORY_STORE,

    NIL_OTHER_INSERT_VALUE,
    NIL_OTHER_LAST,
};

/**
 * @brief Gets opcode name.
 *
 * @param opcode Instruction.
 * @return const char* Opcode name of instruction.
 */
const char *nilGetOpcodeName(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is a terminator instruction.
 *
 * @param opcode Instruction.
 * @return true if opcode is terminator
 * @return false if opcode is terminator
 */
bool nilIsTermiantor(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of unary instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is unary.
 * @return false if opcode is not unary.
 */
bool nilIsUnaryOp(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of binary instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is binary.
 * @return false if opcode is not binary.
 */
bool nilIsBinaryOp(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the div or rem instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a div or rem instruction.
 * @return false if opcode is not a div and rem instruction.
 */
bool nilIsIntDivRem(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the shift instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a shift instruction.
 * @return false if opcode is not a shift instruction.
 */
bool nilIsShift(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the logical shift instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a logical shift instruction.
 * @return false if opcode is not a logical shift instruction.
 */
bool nilIsLogicalShift(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is one of the artihmetic shift instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a arithmetic shift instruction.
 * @return false if opcode is not a arithmetic shift instruction.
 */
bool nilIsArithmeticShift(NIL_OPCODE opcode);

/**
 * @brief Determine if the opcode is and/or/xor.
 *
 * @param opcode Instruction.
 * @return true if opcode is a and/or/xor.
 * @return false if opcode is not a and/or/xor.
 */
bool nilIsBitwiseLogicOp(NIL_OPCODE opcode);

/**
 * @brief Determine if the Opcode is one of the cast instructions.
 *
 * @param opcode Instruction.
 * @return true if opcode is a cast.
 * @return false if opcode is not a cast.
 */
bool nilIsCast(NIL_OPCODE opcode);

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
bool nilIsAssociative(NIL_OPCODE opcode);

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
bool nilIsCommutative(NIL_OPCODE opcode);

/**
 * @brief Determine if the instruction is idempotent.
 *
 * Idempotent operators satisfy: (A op A) === A
 *
 * @param opcode Instruction.
 * @return true if instruction is idempotent.
 * @return false if instruction is idempotent.
 */
bool nilIsIdempotent(NIL_OPCODE opcode);

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
bool nilIsNilpotent(NIL_OPCODE opcode);

NIL_OPCODE nilGetOpcode(NIL_VALUE *instruction);

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
 * @return NIL_VALUE* Value that represents the output of this instruction.
 */
NIL_VALUE *nilCreateBinary(NIL_OPCODE op, NIL_VALUE *source1,
                           NIL_VALUE *source2, const char_t *name,
                           NIL_BASIC_BLOCK *block);

/**
 * @brief Helper for construct the unary NEG operation via binary operator SUB.
 *
 * @param source First operand.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIL_VALUE* Value that represents the output of this instruction.
 */
NIL_VALUE *nilCreateNeg(NIL_VALUE *source, const char_t *name,
                        NIL_BASIC_BLOCK *block);

/**
 * @brief Helper for construct the unary NOT operation via binary operator XOR.
 *
 * @param source First operand.
 * @param name Virtual register name.
 * @param basicBlock Basic block to insert.
 * @return NIL_VALUE* Value that represents the output of this instruction.
 */
NIL_VALUE *nilCreateNot(NIL_VALUE *source, const char_t *name,
                        NIL_BASIC_BLOCK *block);

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
                            NIL_BASIC_BLOCK *block);

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
                            NIL_BASIC_BLOCK *block);

/**
 * @brief Determine if the instruction is uncoditional.
 *
 * @param branch Branch instruction.
 * @return true if is uncoditional branch instruction.
 * @return false if is not uncoditional branch instruction.
 */
bool nilIsUnconditional(NIL_VALUE *branch);

/**
 * @brief Determine if the instruction is coditional.
 *
 * @param branch Branch instruction.
 * @return true if is coditional branch instruction.
 * @return false if is not coditional branch instruction.
 */
bool nilIsConditional(NIL_VALUE *branch);

/**
 * @brief Gets condition value.
 *
 * @param branch Branch instruction.
 * @return NIL_VALUE* condition value of branch instruction, otherwise NULL.
 */
NIL_VALUE *nilGetCondition(NIL_VALUE *branch);

/**
 * @brief Sets condition value.
 *
 * @param branch Branch instruction.
 * @param condition condition value to use in branch.
 */
void nilSetCondition(NIL_VALUE *branch, NIL_VALUE *condition);

/**
 * @brief Gets number of sucessors BasicBlocks.
 *
 * @param term Terminator instruction.
 * @return size_t number of sucessors BasicBlocks.
 */
size_t nilGetSucessorCount(NIL_VALUE *term);

/**
 * @brief Sets a BasicBlock sucessor in a index.
 *
 * @param index Sucessor index, must be bounded by number of sucessors.
 * @param newBasicBlockSucessor New BasicBlock sucessor.
 */
void nilSetSuccessor(NIL_VALUE *branch, size_t index,
                     NIL_BASIC_BLOCK *newBasicBlockSucessor);

/********/
/* Call */
/********/

NIL_VALUE *nilCreateCall(NIL_TYPE *functionType, NIL_FUNCTION *function,
                         size_t argCount, NIL_VALUE **args, const char_t *name,
                         NIL_BASIC_BLOCK *block);

NIL_TYPE *nilGetCallFunctionType(NIL_VALUE *call);
NIL_FUNCTION *nilGetCaller(NIL_VALUE *call);
void nilSetCaller(NIL_VALUE *call, NIL_TYPE *functionType,
                  NIL_FUNCTION *function);
size_t nilGetArgSize(NIL_VALUE *call);
NIL_VALUE *nilGetArgOperand(NIL_VALUE *call, size_t index);
void nilSetArgOperand(NIL_VALUE *call, size_t index, NIL_VALUE *value);

/****************/
/* Compare(CMP) */
/****************/

NT_ENUM(NIL_CMP_PREDICATE){
    /// @brief True if equal
    NIL_FCMP_EQ,
    /// @brief True if greater than
    NIL_FCMP_GT,
    /// @brief True if greater than or equal
    NIL_FCMP_GE,
    /// @brief True if less than
    NIL_FCMP_LT,
    /// @brief True if less than or equal
    NIL_FCMP_LE,
    /// @brief True if operands are unequal
    NIL_FCMP_NE,
    /// @brief True if ordered: (no nan)
    NIL_FCMP_OR,
    /// @brief True if unordered: isnan(x) | isnan(y)
    NIL_FCMP_UO,

    /// @brief True if equal
    NIL_ICMP_EQ,
    /// @brief True if unequal
    NIL_ICMP_NE,

    /// @brief True if unsigned greater than
    NIL_ICMP_UGT,
    /// @brief True if unsigned greater than or equal
    NIL_ICMP_UGE,
    /// @brief True if unsigned less than
    NIL_ICMP_ULT,
    /// @brief True if unsigned less than or equal
    NIL_ICMP_ULE,

    /// @brief True if signed greater than
    NIL_ICMP_SGT,
    /// @brief True if signed greater than or equal
    NIL_ICMP_SGE,
    /// @brief True if signed less than
    NIL_ICMP_SLT,
    /// @brief True if signed less than or equal
    NIL_ICMP_SLE,

    NIL_ICMP_FIRST = NIL_ICMP_EQ,
    NIL_ICMP_LAST = NIL_ICMP_SLE,
    NIL_FCMP_FIRST = NIL_FCMP_EQ,
    NIL_FCMP_LAST = NIL_FCMP_UO,
};

/**
 * @brief Retrieves inverse predicate.
 * For example EQ -> NE, GT -> LE, LT -> GE, OR -> UO, SGT -> SLE, SLT -> SGE,
 * etc.
 *
 * @param predicate Predicate.
 * @return NIL_CMP_PREDICATE opositive predicate.
 */
NIL_CMP_PREDICATE nilInversePredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Retrieves strict version of a non-strict comparison.
 * For example GE -> GT, LE -> LT, UGE -> UGT, ULE -> ULT, SGE -> SGT, SLE ->
 * SLT.
 *
 * @param predicate Predicate.
 * @return NIL_CMP_PREDICATE Strict version.
 */
NIL_CMP_PREDICATE nilStrictPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Retrieves non-strict version of a strict comparison.
 * For example GT -> GE, LT -> LE, UGT -> UGE, ULT -> ULE, SGT -> SGE, SLT ->
 * SLE.
 *
 * @param predicate Predicate.
 * @return NIL_CMP_PREDICATE Non-strict version.
 */
NIL_CMP_PREDICATE nilNonStrictPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Retrieves signed version of a predicate.
 * For example ULT -> SLT, ULE -> SLE, UGT -> SGT, UGE -> SGE, SLT -> failed
 * assert.
 *
 * @param predicate Predicate.
 * @return NIL_CMP_PREDICATE Signed version.
 */
NIL_CMP_PREDICATE nilSignedPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Retrieves unsigned version of a predicate.
 * For example SLT -> ULT, SLE -> ULE, SGT -> UGT, SGE -> UGE, ULT -> failed
 * assert.
 *
 * @param predicate Predicate.
 * @return NIL_CMP_PREDICATE Unsigned version.
 */
NIL_CMP_PREDICATE nilUnsignedPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is a INT predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is a INT predicate.
 * @return false if predicate is not a INT predicate.
 */
bool nilIsIntPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is a FLOAT predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is a FLOAT predicate.
 * @return false if predicate is a FLOAT predicate.
 */
bool nilIsFPPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is strict predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is strict (false when operands are equal).
 * @return false if predicate is not strict.
 */
bool nilIsStrictPredicate(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is non-strict predicate.
 *
 * @param predicate Predicate.
 * @return true if predicate is non-strict (true when operands are equal).
 * @return false if predicate is not non-strict;
 */
bool nilIsEquality(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is relational.
 *
 * @param predicate Predicate.
 * @return true if predicate is relational (not EQ or NE)
 * @return false if predicate is not a relational
 */
bool nilIsRelational(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is signed.
 *
 * @param predicate Predicate.
 * @return true if predicate is signed.
 * @return false if predicate is not a signed.
 */
bool nilIsSigned(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if predicate is unsigned.
 *
 * @param predicate Predicate.
 * @return true if predicate is unsigned.
 * @return false if predicate is not a signed.
 */
bool nilIsUnsigned(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if the predicate is true when comparing a value with itself.
 *
 * @param predicate Predicate.
 * @return true if predicate is true when comparing a value with itself.
 * @return false if predicate is not true when comparing a value with itself.
 */
bool nilIsTrueWhenEqual(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if the predicate is false when comparing a value with
 * itself.
 *
 * @param predicate Predicate.
 * @return true if predicate is false when comparing a value with itself.
 * @return false if predicate is not false when comparing a value with itself.
 */
bool nilIsFalseWhenEqual(NIL_CMP_PREDICATE predicate);

/**
 * @brief Determine if the first condition implies the second is true.
 *
 * @param predicate1 First condition predicate.
 * @param predicate2 Second condition predicate.
 * @return true if first condition implies the second condition is true.
 * @return false if first condition not implies the second condition is true.
 */
bool nilIsImpliedTrueByMatchingCmp(NIL_CMP_PREDICATE predicate1,
                                   NIL_CMP_PREDICATE predicate2);

/**
 * @brief Determine if the first condition implies the second is false.
 *
 * @param predicate1 First codition predicate.
 * @param predicate2 Second condition predicate.
 * @return true if first condition implies the second condition is false.
 * @return false if first condition not implies the second condition is false.
 */
bool nilIsImpliedFalseByMatchingCmp(NIL_CMP_PREDICATE predicate1,
                                    NIL_CMP_PREDICATE predicate2);

/**
 * @brief Gets predicate name;
 *
 * @param predicate Predicate.
 * @return const char* predicate name.
 */
const char *nilGetPredicateName(NIL_CMP_PREDICATE predicate);

/**
 * @brief Gets predicate from a CMP instruction.
 *
 * @param cmp CMP instruction.
 * @return NIL_CMP_PREDICATE predicate of CMP instruction.
 */
NIL_CMP_PREDICATE nilGetPredicate(NIL_VALUE *cmp);

/**
 * @brief Gets inverse predicate from a CMP instruction.
 *
 * @param cmp CMP instruction.
 * @return NIL_CMP_PREDICATE inverse predicate of CMP instruction.
 */
NIL_CMP_PREDICATE nilGetInversePredicate(NIL_VALUE *cmp);

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
 * @return NIL_VALUE* Value that represents the instruction.
 */
NIL_VALUE *nilCreateCmp(NIL_CMP_PREDICATE predicate, NIL_VALUE *source1,
                        NIL_VALUE *source2, const char_t *name,
                        NIL_BASIC_BLOCK *block);

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
 * @return NIL_VALUE* Value that represents the instruction.
 */
NIL_VALUE *nilCreatePhi(NIL_TYPE *valueType, const char_t *name,
                        NIL_BASIC_BLOCK *block);

/**
 * @brief Retrieves the number of incoming values.
 *
 * @param phi PHI node.
 * @return size_t number of incoming values.
 */
size_t nilGetNumIncomingValues(NIL_VALUE *phi);

/**
 * @brief Retrieves the incoming value for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @return NIL_VALUE* incoming value.
 */
NIL_VALUE *nilGetIncomingValue(NIL_VALUE *phi, size_t i);

/**
 * @brief Defines the incoming value for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @param value Incoming value.
 */
void nilSetIncomingValue(NIL_VALUE *phi, size_t i, NIL_VALUE *value);

/**
 * @brief Retrieves the incoming block for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @return NIL_VALUE* incoming block of the index.
 */
NIL_BASIC_BLOCK *nilGetIncomingBlock(NIL_VALUE *phi, size_t i);

/**
 * @brief Defines the incoming block for a incoming value index.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @param basicBlock Incoming block.
 */
void nilSetIncomingBlock(NIL_VALUE *phi, size_t i, NIL_BASIC_BLOCK *block);

/**
 * @brief Add an incoming value to the end of the PHI node.
 *
 * @param phi PHI node.
 * @param value Incoming value.
 * @param basicBlock Incoming block.
 */
void nilAddIncoming(NIL_VALUE *phi, NIL_VALUE *value, NIL_BASIC_BLOCK *block);

/**
 * @brief Remove an incoming value by index. Also automatically
 * destroy this instruction if are empty and replace all usage of this
 * instruction with undefined values.
 *
 * @param phi PHI node.
 * @param i Incoming value index.
 * @return NIL_VALUE* the removed incoming value.
 */
NIL_VALUE *nilRemoveIncomingValue(NIL_VALUE *phi, size_t i);

/**
 * @brief Remove an incoming value by incoming block. Also automatically
 * destroy this instruction if are empty and replace all usage of this
 * instruction with undefined values.
 *
 * @param phi PHI node.
 * @param basicBlock Incoming block.
 * @return NIL_VALUE* the removed incoming value.
 */
NIL_VALUE *nilRemoveIncomingBlock(NIL_VALUE *phi, NIL_BASIC_BLOCK *block);

/**
 * @brief Retrieves the incoming value index of a basic block.
 *
 * @param phi PHI node.
 * @param basicBlock Incoming block.
 * @return size_t incoming value index for a basic block.
 */
size_t nilGetPhiBasicBlockIndex(NIL_VALUE *phi, NIL_BASIC_BLOCK *block);

/**
 * @brief Retrieves the incoming value for a basic block.
 *
 * @param phi PHI node.
 * @param basicBlock Incoming block.
 * @return NIL_VALUE* incoming value for the basic block, otherwise NULL.
 */
NIL_VALUE *nilGetIncomingValueForBlock(NIL_VALUE *phi, NIL_BASIC_BLOCK *block);

/**
 * @brief Determine if the PHI node always merges together the same value.
 *
 * @param phi PHI node.
 * @return NIL_VALUE* the same value, otherwise NULL.
 */
NIL_VALUE *nilPhiHasConstantValue(NIL_VALUE *phi);

/**
 * @brief Determine if the PHI node always mergers together the same value,
 * assuming undefined values are the same value as non-undefined values.
 *
 * @param phi PHI node.
 * @return true If all defined values merges to same value.
 * @return false If any defined value do not merge to same value.
 */
bool nilPhiHasConstantOrUndefValue(NIL_VALUE *phi);

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
bool nilPhiIsComplete(NIL_VALUE *phi);

/**********/
/* Return */
/**********/

/**
 * @brief Construct a return instruction, given return value. Also automatically
 * insert this instruction to the end of the BasicBlock specified.
 *
 * @param context NIL_CONTEXT
 * @param returnValue The value that will be returned.
 * @param basicBlock Basic block to insert.
 * @return NIL_VALUE* Value that represents the instruction.
 */
NIL_VALUE *nilCreateReturn(NIL_VALUE *returnValue, NIL_BASIC_BLOCK *block);

/**
 * @brief Retrieves the return value of a return instruction.
 *
 * @param ret Return instruction.
 * @return NIL_VALUE* The return value of the return instruction, otherwise
 * NULL.
 */
NIL_VALUE *nilGetReturnValue(NIL_VALUE *ret);

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
 * @return NIL_VALUE* Value that represents the instruction.
 */
NIL_VALUE *nilCreateSelect(NIL_VALUE *condition, NIL_VALUE *valueWhenTrue,
                           NIL_VALUE *valueWhenFalse, const char_t *name,
                           NIL_BASIC_BLOCK *block);

/**
 * @brief Retrieves the condition value of a select instruction.
 *
 * @param select Select instruction.
 * @return NIL_VALUE* condition value.
 */
NIL_VALUE *nilGetCondition(NIL_VALUE *select);

/**
 * @brief Retrieves the true value of a select instruction.
 *
 * @param select Select instruction.
 * @return NIL_VALUE* true value.
 */
NIL_VALUE *nilGetTrueValue(NIL_VALUE *select);

/**
 * @brief Retrieves the false value of a select instruction.
 *
 * @param select Select instruction.
 * @return NIL_VALUE* false value.
 */
NIL_VALUE *nilGetFalseValue(NIL_VALUE *select);

/**
 * @brief Defines the condition value of a select instruction.
 *
 * @param select Select instruction.
 * @param value Condition value.
 */
void nilSetCondition(NIL_VALUE *select, NIL_VALUE *condition);

/**
 * @brief Defines the true value of a select instruction.
 *
 * @param select Select instruction.
 * @param value True value.
 */
void nilSetTrueValue(NIL_VALUE *select, NIL_VALUE *value);

/**
 * @brief Defines the false value of a select instruction.
 *
 * @param select Select instruction.
 * @param value False value.
 */
void nilSetFalseValue(NIL_VALUE *select, NIL_VALUE *value);

/**
 * @brief Swap the true and false values of the select instruction.
 *
 * @param select Select instruction.
 */
void nilSwapValues(NIL_VALUE *select);

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
const char *nilAreSelectInvalidOperands(NIL_VALUE *condition,
                                        NIL_VALUE *trueValue,
                                        NIL_VALUE *falseValue);

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
                          NIL_BASIC_BLOCK *block);

/**
 * @brief Retrieves the pointer operand of store instruction.
 *
 * @param store Store instruction.
 * @return NIL_VALUE* that is the pointer operand.
 */
NIL_VALUE *nilGetPointerOperand(NIL_VALUE *store);

/**
 * @brief Retrieves the pointer operand type of store instruction.
 *
 * @param store Store instruction.
 * @return NIL_TYPE* that is the type of pointer operand.
 */
NIL_TYPE *nilGetPointerOperandType(NIL_VALUE *store);

/**
 * @brief Retrieves the value operand of store instruction.
 *
 * @param store Store instruction.
 * @return NIL_VALUE* that is the value operand.
 */
NIL_VALUE *nilGetValueOperand(NIL_VALUE *store);

/**********/
/* Unary */
/**********/

NIL_VALUE *nilCreateUnaryInst(NIL_OPCODE unaryOp, NIL_TYPE *valueType,
                              NIL_VALUE *value, const char_t *name,
                              NIL_BASIC_BLOCK *block);

NIL_VALUE *nilGetUnaryValueOperand(NIL_VALUE *unary);
NIL_TYPE *nilGetUnaryTypeOperand(NIL_VALUE *unary);

#endif
