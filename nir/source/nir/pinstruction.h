#ifndef NIR_PINSTRUCTION_H
#define NIR_PINSTRUCTION_H

#include "netuno/common.h"
#include "netuno/nir/value.h"
#include "plist.h"
#include "ptype.h"
#include "pvalue.h"
#include <netuno/nir/instruction.h>

#define INST_FIELDS                                                            \
    union {                                                                    \
        struct /* value */                                                     \
        {                                                                      \
            NT_STRING *name;                                                   \
            NIR_VALUE_TYPE valueType;                                          \
            NIR_DEBUG_LOC *dbgLoc;                                             \
            NIR_TYPE *type;                                                    \
        };                                                                     \
        struct /* instruction */                                               \
        {                                                                      \
            NIR_VALUE value;                                                   \
            NIR_OPCODE opcode;                                                 \
            NIR_BASIC_BLOCK *parent;                                           \
        };                                                                     \
    }

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_INSTRUCTION)
struct _NIR_INSTRUCTION
{
    INST_FIELDS;
};

#define TO_INST(value)                                                         \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    NIR_INSTRUCTION *inst = (NIR_INSTRUCTION *)(value)

NT_HANDLE(NIR_BINARY_OPERATOR)
struct _NIR_BINARY_OPERATOR
{
    INST_FIELDS;

    NIR_VALUE *source1;
    NIR_VALUE *source2;
};

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_BRANCH_INST)
struct _NIR_BRANCH_INST
{
    INST_FIELDS;

    NIR_BASIC_BLOCK *ifTrue;
    NIR_BASIC_BLOCK *ifFalse;
    NIR_VALUE *condition;
};
#define TO_BR(value)                                                           \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_TERM_BR);               \
    NIR_BRANCH_INST *br = (NIR_BRANCH_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_CALL_INST)
struct _NIR_CALL_INST
{
    INST_FIELDS;

    NIR_FUNCTION_TYPE *functionType;
    NIR_FUNCTION *function;
    size_t numArgs;
    NIR_VALUE **arguments;
};
#define TO_CALL(value)                                                         \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_OTHER_CALL);            \
    NIR_CALL_INST *call = (NIR_CALL_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_CMP_INST)
struct _NIR_CMP_INST
{
    INST_FIELDS;

    NIR_VALUE *source1;
    NIR_VALUE *source2;
    NIR_CMP_PREDICATE predicate;
};
#define TO_CMP(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_OTHER_CMP);             \
    NIR_CMP_INST *cmp = (NIR_CMP_INST *)(value)

NT_HANDLE(NIR_INCOMING_VALUE)
struct _NIR_INCOMING_VALUE
{
    NIR_VALUE *value;
    NIR_BASIC_BLOCK *block;
};

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_PHI_NODE)
struct _NIR_PHI_NODE
{
    INST_FIELDS;

    LIST(NIR_INCOMING_VALUE *, incomingValues, list);
};
#define TO_PHI(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_OTHER_PHI);             \
    NIR_PHI_NODE *phi = (NIR_PHI_NODE *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_RETURN_INST)
struct _NIR_RETURN_INST
{
    INST_FIELDS;

    NIR_VALUE *retValue;
};
#define TO_RET(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_TERM_RET);              \
    NIR_RETURN_INST *ret = (NIR_RETURN_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_SELECT_INST)
struct _NIR_SELECT_INST
{
    INST_FIELDS;

    /// @brief A i1 value indicating the condition.
    NIR_VALUE *condition;
    /// @brief The result value when condition evaluates to 1.
    NIR_VALUE *trueValue;
    /// @brief The result value when condition evaluates to 0.
    NIR_VALUE *falseValue;
};

#define TO_SEL(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_OTHER_SELECT);          \
    NIR_SELECT_INST *sel = (NIR_SELECT_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_STORE_INST)
struct _NIR_STORE_INST
{
    INST_FIELDS;

    /// @brief Value to write.
    NIR_VALUE *source;
    /// @brief Pointer to write address.
    NIR_VALUE *ptr;
};
#define TO_STR(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(((NIR_INSTRUCTION *)(value))->opcode == NIR_OTHER_MEMORY_STORE);    \
    NIR_STORE_INST *str = (NIR_STORE_INST *)(value)

////////////////////////////////////////////////////////////////////////////////

NT_HANDLE(NIR_UNARY_OPERATOR)
struct _NIR_UNARY_OPERATOR
{
    INST_FIELDS;

    NIR_VALUE *source;
};
#define TO_UNA(value)                                                          \
    assert(value);                                                             \
    assert((value)->valueType == NIR_VALUE_TYPE_INSTRUCTION);                  \
    assert(nirIsUnaryOp(((NIR_INSTRUCTION *)(value))->opcode));                \
    NIR_UNARY_OPERATOR *una = (NIR_UNARY_OPERATOR *)(value)

#endif
