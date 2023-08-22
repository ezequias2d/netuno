#ifndef NIL_PINSTRUCTION_H
#define NIL_PINSTRUCTION_H

#include "netuno/common.h"
#include "netuno/nil/value.h"
#include "plist.h"
#include "ptype.h"
#include "pvalue.h"
#include <netuno/nil/instruction.h>

#define INST_FIELDS                                                            \
    union {                                                                    \
        struct /* value */                                                     \
        {                                                                      \
            NT_STRING *name;                                                   \
            NIL_VALUE_TYPE valueType;                                          \
            NIL_DEBUG_LOC *dbgLoc;                                             \
            NIL_TYPE *type;                                                    \
        };                                                                     \
        struct /* instruction */                                               \
        {                                                                      \
            NIL_VALUE value;                                                   \
            NIL_OPCODE opcode;                                                 \
            NIL_BASIC_BLOCK *parent;                                           \
        };                                                                     \
    }

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_INSTRUCTION)
struct _NIL_INSTRUCTION
{
    INST_FIELDS;
};

#define TO_INST(value)                                                         \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    NIL_INSTRUCTION *inst = (NIL_INSTRUCTION *)(value)

NT_HANDLE(NIL_BINARY_OPERATOR)
struct _NIL_BINARY_OPERATOR
{
    INST_FIELDS;

    NIL_VALUE *source1;
    NIL_VALUE *source2;
};

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_BRANCH_INST)
struct _NIL_BRANCH_INST
{
    INST_FIELDS;

    NIL_BASIC_BLOCK *ifTrue;
    NIL_BASIC_BLOCK *ifFalse;
    NIL_VALUE *condition;
};
#define TO_BR(value)                                                           \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_TERM_BR);               \
    NIL_BRANCH_INST *br = (NIL_BRANCH_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_CALL_INST)
struct _NIL_CALL_INST
{
    INST_FIELDS;

    NIL_FUNCTION_TYPE *functionType;
    NIL_FUNCTION *function;
    size_t numArgs;
    NIL_VALUE **arguments;
};
#define TO_CALL(value)                                                         \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_OTHER_CALL);            \
    NIL_CALL_INST *call = (NIL_CALL_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_CMP_INST)
struct _NIL_CMP_INST
{
    INST_FIELDS;

    NIL_VALUE *source1;
    NIL_VALUE *source2;
    NIL_CMP_PREDICATE predicate;
};
#define TO_CMP(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_OTHER_CMP);             \
    NIL_CMP_INST *cmp = (NIL_CMP_INST *)(value)

NT_HANDLE(NIL_INCOMING_VALUE)
struct _NIL_INCOMING_VALUE
{
    NIL_VALUE *value;
    NIL_BASIC_BLOCK *block;
};

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_PHI_NODE)
struct _NIL_PHI_NODE
{
    INST_FIELDS;

    LIST(NIL_INCOMING_VALUE *, incomingValues, list);
};
#define TO_PHI(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_OTHER_PHI);             \
    NIL_PHI_NODE *phi = (NIL_PHI_NODE *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_RETURN_INST)
struct _NIL_RETURN_INST
{
    INST_FIELDS;

    NIL_VALUE *retValue;
};
#define TO_RET(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_TERM_RET);              \
    NIL_RETURN_INST *ret = (NIL_RETURN_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_SELECT_INST)
struct _NIL_SELECT_INST
{
    INST_FIELDS;

    /// @brief A i1 value indicating the condition.
    NIL_VALUE *condition;
    /// @brief The result value when condition evaluates to 1.
    NIL_VALUE *trueValue;
    /// @brief The result value when condition evaluates to 0.
    NIL_VALUE *falseValue;
};

#define TO_SEL(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_OTHER_SELECT);          \
    NIL_SELECT_INST *sel = (NIL_SELECT_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_STORE_INST)
struct _NIL_STORE_INST
{
    INST_FIELDS;

    /// @brief Value to write.
    NIL_VALUE *source;
    /// @brief Pointer to write address.
    NIL_VALUE *ptr;
};
#define TO_STR(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIL_INSTRUCTION *)(value))->opcode == NIL_OTHER_MEMORY_STORE);    \
    NIL_STORE_INST *str = (NIL_STORE_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIL_UNARY_OPERATOR)
struct _NIL_UNARY_OPERATOR
{
    INST_FIELDS;

    NIL_VALUE *source;
};
#define TO_UNA(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIL_VALUE_TYPE_INSTRUCTION);                  \
    assert(nilIsUnaryOp(((NIL_INSTRUCTION *)(value))->opcode));                \
    NIL_UNARY_OPERATOR *una = (NIL_UNARY_OPERATOR *)(value)

#endif
