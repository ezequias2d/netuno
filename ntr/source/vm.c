#include <assert.h>
#include <float.h>
#include <math.h>
#include <netuno/debug.h>
#include <netuno/memory.h>
#include <netuno/object.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/vm.h>
#include <stdio.h>

NT_VM *ntCreateVM(void)
{
    NT_VM *vm = (NT_VM *)ntMalloc(sizeof(NT_VM));
    vm->stack = ntMalloc(STACK_MAX);
    vm->stackTop = vm->stack;
    vm->gc = ntCreateGarbageCollector();
#ifdef DEBUG_TRACE_EXECUTION
    vm->stackType = (size_t *)ntMalloc(sizeof(size_t) * STACK_MAX);
    vm->stackTypeTop = vm->stackType;
#endif
    return vm;
}

void ntFreeVM(NT_VM *vm)
{
    ntFreeGarbageCollector(vm->gc);
#ifdef DEBUG_TRACE_EXECUTION
    ntFree(vm->stackType);
#endif
    ntFree(vm->stack);
    ntFree(vm);
}

void ntResetStack(NT_VM *vm)
{
    vm->stackTop = vm->stack;
    vm->stackOverflow = false;
#ifdef DEBUG_TRACE_EXECUTION
    vm->stackTypeTop = vm->stackType;
#endif
}

bool ntPush(NT_VM *vm, const void *data, const size_t dataSize)
{
    const size_t available = STACK_MAX - (vm->stackTop - vm->stack);
    if (available < dataSize)
    {
        vm->stackOverflow = true;
        return false;
    }
    ntMemcpy(vm->stackTop, data, dataSize);
    vm->stackTop += dataSize;

#ifdef DEBUG_TRACE_EXECUTION
    ntMemcpy(vm->stackTypeTop, &dataSize, sizeof(size_t));
    vm->stackTypeTop++;
#endif
    return true;
}

bool ntPeek(NT_VM *vm, void *data, const size_t dataSize, size_t offset)
{
    const size_t available = vm->stackTop - vm->stack;

    if (available <= offset)
        return false;
    if (available - offset < dataSize)
        return false;

    ntMemcpy(data, vm->stackTop - dataSize - offset, dataSize);
    return true;
}

static bool ntWriteSp(NT_VM *vm, const void *data, const size_t dataSize, size_t offset)
{
    const size_t available = vm->stackTop - vm->stack;

    if (available <= offset)
        return false;
    if (available - offset < dataSize)
        return false;

    ntMemcpy(vm->stackTop - dataSize - offset, data, dataSize);
    return true;
}

bool ntPop(NT_VM *vm, void *data, const size_t dataSize)
{
    const size_t available = vm->stackTop - vm->stack;
    if (available < dataSize)
    {
        vm->stackOverflow = true;
        return false;
    }
    vm->stackTop -= dataSize;
    ntMemcpy(data, vm->stackTop, dataSize);
#ifdef DEBUG_TRACE_EXECUTION
    vm->stackTypeTop--;
#endif
    return true;
}

bool ntPush32(NT_VM *vm, const uint32_t value)
{
    return ntPush(vm, &value, sizeof(uint32_t));
}

bool ntPop32(NT_VM *vm, uint32_t *value)
{
    return ntPop(vm, value, sizeof(uint32_t));
}

bool ntPush64(NT_VM *vm, const uint64_t value)
{
    return ntPush(vm, &value, sizeof(uint64_t));
}

bool ntPop64(NT_VM *vm, uint64_t *value)
{
    return ntPop(vm, value, sizeof(uint64_t));
}

bool ntPopObject(NT_VM *vm, NT_OBJECT **object)
{
    uint32_t ref;
    const bool result = ntPop32(vm, &ref);
    *object = ntGetObject(vm->gc, ref);
    return result;
}

bool ntPushObject(NT_VM *vm, NT_OBJECT *object)
{
    const uint32_t ref = ntAddObject(vm->gc, object);
    return ntPush32(vm, ref);
}

static uint32_t readConst32(NT_VM *vm)
{
    uint64_t constant;
    vm->pc += ntReadVariant(vm->chunk, vm->pc, &constant);

    uint32_t value;
    assert(ntArrayGetU32(&vm->chunk->constants, constant, &value));
    return value;
}

static uint64_t readConst64(NT_VM *vm)
{
    uint64_t constant;
    vm->pc += ntReadVariant(vm->chunk, vm->pc, &constant);

    uint64_t value;
    assert(ntArrayGetU64(&vm->chunk->constants, constant, &value));
    return value;
}

static const NT_STRING *readConstString(NT_VM *vm)
{
    uint64_t constant;
    vm->pc += ntReadVariant(vm->chunk, vm->pc, &constant);

    char_t *chars;
    size_t length;
    ntArrayGetString(&vm->chunk->constants, constant, NULL, &length);

    chars = (char_t *)ntMalloc(sizeof(char_t) * (length + 1));
    ntArrayGetString(&vm->chunk->constants, constant, chars, &length);
    chars[length] = '\0';

    const NT_STRING *str = ntTakeString(chars, length);

    return str;
}

static void printHex(const uint8_t *data, const size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        printf("%02X", data[i]);
    }
}

static uint32_t negate32(const uint32_t value)
{
    return -value;
}

static uint64_t negate64(const uint64_t value)
{
    return -value;
}

static uint32_t negateF32(const uint32_t value)
{
    const float result = -*(float *)&value;
    return *(uint32_t *)&result;
}

static uint64_t negateF64(const uint64_t value)
{
    const double result = -*(double *)&value;
    return *(uint64_t *)&result;
}

static uint32_t add32(const uint32_t value1, const uint32_t value2)
{
    return value1 + value2;
}

static uint64_t add64(const uint64_t value1, const uint64_t value2)
{
    return value1 + value2;
}

static uint32_t addF32(const uint32_t value1, const uint32_t value2)
{
    const float result = *(float *)&value1 + *(float *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t addF64(const uint64_t value1, const uint64_t value2)
{
    const double result = *(double *)&value1 + *(double *)&value2;
    return *(uint64_t *)&result;
}

static uint32_t sub32(const uint32_t value1, const uint32_t value2)
{
    return value1 - value2;
}

static uint64_t sub64(const uint64_t value1, const uint64_t value2)
{
    return value1 - value2;
}

static uint32_t subF32(const uint32_t value1, const uint32_t value2)
{
    const float result = *(float *)&value1 - *(float *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t subF64(const uint64_t value1, const uint64_t value2)
{
    const double result = *(double *)&value1 - *(double *)&value2;
    return *(uint64_t *)&result;
}

static uint32_t mul32(const uint32_t value1, const uint32_t value2)
{
    return value1 * value2;
}

static uint64_t mul64(const uint64_t value1, const uint64_t value2)
{
    return value1 * value2;
}

static uint32_t mulF32(const uint32_t value1, const uint32_t value2)
{
    const float result = *(float *)&value1 * *(float *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t mulF64(const uint64_t value1, const uint64_t value2)
{
    const double result = *(double *)&value1 * *(double *)&value2;
    return *(uint64_t *)&result;
}

static uint32_t divU32(const uint32_t value1, const uint32_t value2)
{
    return value1 / value2;
}

static uint64_t divU64(const uint64_t value1, const uint64_t value2)
{
    return value1 / value2;
}

static uint32_t divI32(const uint32_t value1, const uint32_t value2)
{
    const int32_t result = *(int32_t *)&value1 / *(int32_t *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t divI64(const uint64_t value1, const uint64_t value2)
{
    const int64_t result = *(int64_t *)&value1 / *(int64_t *)&value2;
    return *(uint64_t *)&result;
}

static uint32_t divF32(const uint32_t value1, const uint32_t value2)
{
    const float result = *(float *)&value1 / *(float *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t divF64(const uint64_t value1, const uint64_t value2)
{
    const double result = *(double *)&value1 / *(double *)&value2;
    return *(uint64_t *)&result;
}
static uint32_t remU32(const uint32_t value1, const uint32_t value2)
{
    return value1 % value2;
}

static uint64_t remU64(const uint64_t value1, const uint64_t value2)
{
    return value1 % value2;
}

static uint32_t remI32(const uint32_t value1, const uint32_t value2)
{
    const int32_t result = *(int32_t *)&value1 % *(int32_t *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t remI64(const uint64_t value1, const uint64_t value2)
{
    const int64_t result = *(int64_t *)&value1 % *(int64_t *)&value2;
    return *(uint64_t *)&result;
}

static uint32_t remF32(const uint32_t value1, const uint32_t value2)
{
    const float result = fmodf(*(float *)&value1, *(float *)&value2);
    return *(uint32_t *)&result;
}

static uint64_t remF64(const uint64_t value1, const uint64_t value2)
{
    const double result = fmod(*(double *)&value1, *(double *)&value2);
    return *(uint64_t *)&result;
}

static uint64_t extendI32(const uint32_t value)
{
    const int64_t result = (int64_t) * (int32_t *)&value;
    return *(uint64_t *)&result;
}

static uint64_t extendU32(const uint64_t value)
{
    return (uint64_t) * (uint32_t *)&value;
}

static uint64_t promoteF32(const uint32_t value)
{
    const double result = (double)*(float *)&value;
    return *(uint64_t *)&result;
}

static uint32_t demoteF64(const uint64_t value)
{
    const float result = *(float *)&value;
    return *(uint32_t *)&result;
}

static uint32_t convertI32ToF32(const uint32_t value)
{
    const float result = (float)*(int32_t *)&value;
    return *(uint32_t *)&result;
}

static uint64_t convertI32ToF64(const uint32_t value)
{
    const double result = (double)*(int32_t *)&value;
    return *(uint64_t *)&result;
}

static uint32_t convertU32ToF32(const uint32_t value)
{
    const float result = (float)*(uint32_t *)&value;
    return *(uint32_t *)&result;
}

static uint64_t convertU32ToF64(const uint32_t value)
{
    const double result = (double)*(uint32_t *)&value;
    return *(uint64_t *)&result;
}

static uint32_t convertI64ToF32(const uint64_t value)
{
    const float result = (float)*(int64_t *)&value;
    return *(uint32_t *)&result;
}

static uint64_t convertI64ToF64(const uint64_t value)
{
    const double result = (double)*(int64_t *)&value;
    return *(uint64_t *)&result;
}

static uint32_t convertU64ToF32(const uint64_t value)
{
    const float result = (float)*(uint64_t *)&value;
    return *(uint32_t *)&result;
}

static uint64_t convertU64ToF64(const uint64_t value)
{
    const double result = (double)*(uint64_t *)&value;
    return *(uint64_t *)&result;
}

static uint32_t truncateFloatToI32(const uint32_t value)
{
    const int32_t result = (int32_t) * (float *)&value;
    return *(uint32_t *)&result;
}

static uint32_t truncateDoubleToI32(const uint64_t value)
{
    const int32_t result = (int32_t) * (double *)&value;
    return *(uint32_t *)&result;
}

static uint32_t truncateFloatToU32(const uint32_t value)
{
    return (uint32_t) * (float *)&value;
}

static uint32_t truncateDoubleToU32(const uint64_t value)
{
    return (uint32_t) * (double *)&value;
}

static uint64_t truncateFloatToI64(const uint32_t value)
{
    const int64_t result = (int64_t) * (float *)&value;
    return *(uint64_t *)&result;
}

static uint64_t truncateDoubleToI64(const uint64_t value)
{
    const int64_t result = (int64_t) * (double *)&value;
    return *(uint64_t *)&result;
}

static uint64_t truncateFloatToU64(const uint32_t value)
{
    return (uint64_t) * (float *)&value;
}

static uint64_t truncateDoubleToU64(const uint64_t value)
{
    return (uint64_t) * (double *)&value;
}

static uint32_t minF32(const uint32_t value1, const uint32_t value2)
{
    const float result = fminf(*(float *)&value1, *(float *)&value2);
    return *(uint32_t *)&result;
}

static uint64_t minF64(const uint64_t value1, const uint64_t value2)
{
    const double result = fmin(*(double *)&value1, *(double *)&value2);
    return *(uint64_t *)&result;
}

static uint32_t maxF32(const uint32_t value1, const uint32_t value2)
{
    const float result = fmaxf(*(float *)&value1, *(float *)&value2);
    return *(uint32_t *)&result;
}

static uint64_t maxF64(const uint64_t value1, const uint64_t value2)
{
    const double result = fmax(*(double *)&value1, *(double *)&value2);
    return *(uint64_t *)&result;
}

static uint32_t nearestF32(const uint32_t value)
{
    const float result = roundf(*(float *)&value);
    return *(uint32_t *)&result;
}

static uint64_t nearestF64(const uint64_t value)
{
    const double result = round(*(double *)&value);
    return *(uint64_t *)&result;
}

static uint32_t ceilF32(const uint32_t value)
{
    const float result = ceilf(*(float *)&value);
    return *(uint32_t *)&result;
}

static uint64_t ceilF64(const uint64_t value)
{
    const double result = ceil(*(double *)&value);
    return *(uint64_t *)&result;
}

static uint32_t floorF32(const uint32_t value)
{
    const float result = floorf(*(float *)&value);
    return *(uint32_t *)&result;
}

static uint64_t floorF64(const uint64_t value)
{
    const double result = floor(*(double *)&value);
    return *(uint64_t *)&result;
}

static uint32_t truncateF32(const uint32_t value)
{
    const float result = truncf(*(float *)&value);
    return *(uint32_t *)&result;
}

static uint64_t truncateF64(const uint64_t value)
{
    const double result = trunc(*(double *)&value);
    return *(uint64_t *)&result;
}

static uint32_t absF32(const uint32_t value)
{
    const float result = fabsf(*(float *)&value);
    return *(uint32_t *)&result;
}

static uint64_t absF64(const uint64_t value)
{
    const double result = fabs(*(double *)&value);
    return *(uint64_t *)&result;
}

static uint32_t sqrtF32(const uint32_t value)
{
    const float result = sqrtf(*(float *)&value);
    return *(uint32_t *)&result;
}

static uint64_t sqrtF64(const uint64_t value)
{
    const double result = sqrt(*(double *)&value);
    return *(uint64_t *)&result;
}

static uint32_t copysignF32(const uint32_t value1, const uint32_t value2)
{
    const float result = copysignf(*(float *)&value1, *(float *)&value2);
    return *(uint32_t *)&result;
}

static uint64_t copysignF64(const uint64_t value1, const uint64_t value2)
{
    const double result = copysign(*(double *)&value1, *(double *)&value2);
    return *(uint64_t *)&result;
}

static uint32_t and32(const uint32_t value1, const uint32_t value2)
{
    return value1 & value2;
}

static uint64_t and64(const uint64_t value1, const uint64_t value2)
{
    return value1 & value2;
}

static uint32_t or32(const uint32_t value1, const uint32_t value2)
{
    return value1 | value2;
}

static uint64_t or64(const uint64_t value1, const uint64_t value2)
{
    return value1 | value2;
}

static uint32_t xor32(const uint32_t value1, const uint32_t value2)
{
    return value1 ^ value2;
}

static uint64_t xor64(const uint64_t value1, const uint64_t value2)
{
    return value1 ^ value2;
}

static uint32_t shl32(const uint32_t value1, const uint32_t value2)
{
    return value1 << value2;
}

static uint64_t shl64(const uint64_t value1, const uint64_t value2)
{
    return value1 << value2;
}

static uint32_t shrU32(const uint32_t value1, const uint32_t value2)
{
    return value1 >> value2;
}

static uint64_t shrU64(const uint64_t value1, const uint64_t value2)
{
    return value1 >> value2;
}

static uint32_t shrS32(const uint32_t value1, const uint32_t value2)
{
    const int32_t result = *(int32_t *)&value1 >> *(int32_t *)&value2;
    return *(uint32_t *)&result;
}

static uint64_t shrS64(const uint64_t value1, const uint64_t value2)
{
    const int64_t result = *(int64_t *)&value1 >> *(int64_t *)&value2;
    return *(uint64_t *)&result;
}

static uint32_t rol32(const uint32_t value1, const uint32_t value2)
{
    return (value1 << value2) | (value1 >> (sizeof(value1) * 8 - value2));
}

static uint64_t rol64(const uint64_t value1, const uint64_t value2)
{
    return (value1 << value2) | (value1 >> (sizeof(value1) * 8 - value2));
}

static uint32_t ror32(const uint32_t value1, const uint32_t value2)
{
    return (value1 >> value2) | (value1 << (sizeof(value1) * 8 - value2));
}

static uint64_t ror64(const uint64_t value1, const uint64_t value2)
{
    return (value1 >> value2) | (value1 << (sizeof(value1) * 8 - value2));
}

static uint32_t clz32(const uint32_t value)
{
    return (uint32_t)__builtin_clz(value);
}

static uint64_t clz64(const uint64_t value)
{
    return (uint64_t)__builtin_clzl(value);
}

static uint32_t ctz32(const uint32_t value)
{
    return (uint32_t)__builtin_ctz(value);
}

static uint64_t ctz64(const uint64_t value)
{
    return (uint64_t)__builtin_ctzl(value);
}

static uint32_t popcount32(const uint32_t value)
{
    return (uint32_t)__builtin_popcount(value);
}

static uint64_t popcount64(const uint64_t value)
{
    return (uint64_t)__builtin_popcountl(value);
}

static NT_RESULT run(NT_VM *vm)
{
    for (;;)
    {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        size_t debugOffset = 0;
        for (size_t *i = vm->stackType; i < vm->stackTypeTop; ++i)
        {
            printf("[");
            printHex(vm->stack + debugOffset, *i);
            printf("]");
            debugOffset += *i;
        }
        printf("\n");
        ntDisassembleInstruction(vm->chunk, vm->pc);
#endif
        uint8_t instruction;
        uint64_t t64_1;
        uint64_t t64_2;
        uint64_t t64_3;
        uint32_t t32_1;
        uint32_t t32_2;
        bool result;
        switch (instruction = ntRead(vm->chunk, vm->pc++))
        {
        case BC_PRINT: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            char *s = ntToChar(str->chars);
            printf("%s\n", s);
            ntFree(s);
        }
        break;

        case BC_BRANCH:
            // t64_1 = offset between current instruction and target
            // t64_2 = bytes used by offset in current instruction
            // because current instruction is in PC - 1
            // target offset is t64_1 + t64_2 - 1
            // this is same for BRANCH_Z_32 and BRANCH_Z_64 instructions
            t64_2 = ntReadVariant(vm->chunk, vm->pc, &t64_1);
            vm->pc += t64_1 + t64_2 - 1;
            break;
        case BC_BRANCH_Z_32:
            t64_2 = ntReadVariant(vm->chunk, vm->pc, &t64_1);
            if (!ntPeek(vm, &t32_1, sizeof(uint32_t), 0))
            {
                printf("Empty stack!");
                assert(false);
                break;
            }

            if (t32_1 == 0)
                vm->pc += t64_1 + t64_2 - 1;
            else
                vm->pc += t64_2;
            break;
        case BC_BRANCH_Z_64:
            t64_2 = ntReadVariant(vm->chunk, vm->pc, &t64_1);
            if (!ntPeek(vm, &t64_3, sizeof(uint32_t), vm->pc))
            {
                printf("Empty stack!");
                assert(false);
                break;
            }

            if (t64_3 == 0)
                vm->pc += t64_1 + t64_2 - 1;
            else
                vm->pc += t64_2;
            break;

        case BC_ZERO_32:
            result = ntPush32(vm, 0);
            assert(result);
            break;
        case BC_ONE_32:
            result = ntPush32(vm, 1);
            assert(result);
            break;
        case BC_CONST_32:
            t32_1 = readConst32(vm);
            result = ntPush32(vm, t32_1);
            assert(result);
            break;
        case BC_CONST_64:
            t64_1 = readConst64(vm);
            result = ntPush64(vm, t64_1);
            assert(result);
            break;
        case BC_CONST_STRING: {
            const NT_STRING *str = readConstString(vm);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_LOAD_SP_32:
            vm->pc += ntReadVariant(vm->chunk, vm->pc, &t64_1);

            result = ntPeek(vm, &t32_1, sizeof(uint32_t), t64_1);
            assert(result);
            result = ntPush32(vm, t32_1);
            assert(result);
            break;
        case BC_LOAD_SP_64:
            vm->pc += ntReadVariant(vm->chunk, vm->pc, &t64_1);

            result = ntPeek(vm, &t64_1, sizeof(uint64_t), t64_1);
            assert(result);
            result = ntPush32(vm, t64_1);
            assert(result);
            break;
        case BC_STORE_SP_32:
            vm->pc += ntReadVariant(vm->chunk, vm->pc, &t64_1);
            result = ntPeek(vm, &t32_1, sizeof(uint32_t), 0);
            assert(result);
            result = ntWriteSp(vm, &t32_1, sizeof(uint32_t), t64_1);
            assert(result);
            break;
        case BC_STORE_SP_64:
            vm->pc += ntReadVariant(vm->chunk, vm->pc, &t64_1);
            result = ntPeek(vm, &t64_2, sizeof(uint64_t), 0);
            assert(result);
            result = ntWriteSp(vm, &t64_2, sizeof(uint64_t), t64_1);
            assert(result);
            break;

        case BC_EQ_32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, t32_1 == t32_2);
            assert(result);
            break;
        case BC_EQ_64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, t64_1 == t64_2);
            assert(result);
            break;
        case BC_EQ_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(float *)&t32_1 == *(float *)&t32_2);
            assert(result);
            break;
        case BC_EQ_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(double *)&t64_1 == *(double *)&t64_2);
            assert(result);
            break;

        case BC_NE_32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, t32_1 != t32_2);
            assert(result);
            break;
        case BC_NE_64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, t64_1 != t64_2);
            assert(result);
            break;
        case BC_NE_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(float *)&t32_1 != *(float *)&t32_2);
            assert(result);
            break;
        case BC_NE_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(double *)&t64_1 != *(double *)&t64_2);
            assert(result);
            break;

        case BC_GT_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(int32_t *)&t32_1 > *(int32_t *)&t32_2);
            assert(result);
            break;
        case BC_GT_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, t32_1 > t32_2);
            assert(result);
            break;
        case BC_GT_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(int64_t *)&t64_1 > *(int64_t *)&t64_2);
            assert(result);
            break;
        case BC_GT_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, t64_1 > t64_2);
            assert(result);
            break;
        case BC_GT_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(float *)&t32_1 > *(float *)&t32_2);
            assert(result);
            break;
        case BC_GT_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(double *)&t64_1 > *(double *)&t64_2);
            assert(result);
            break;

        case BC_LT_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(int32_t *)&t32_1 < *(int32_t *)&t32_2);
            assert(result);
            break;
        case BC_LT_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, t32_1 < t32_2);
            assert(result);
            break;
        case BC_LT_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(int64_t *)&t64_1 < *(int64_t *)&t64_2);
            assert(result);
            break;
        case BC_LT_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, t64_1 < t64_2);
            assert(result);
            break;
        case BC_LT_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(float *)&t32_1 < *(float *)&t32_2);
            assert(result);
            break;
        case BC_LT_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(double *)&t64_1 < *(double *)&t64_2);
            assert(result);
            break;

        case BC_GE_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(int32_t *)&t32_1 >= *(int32_t *)&t32_2);
            assert(result);
            break;
        case BC_GE_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, t32_1 >= t32_2);
            assert(result);
            break;
        case BC_GE_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(int64_t *)&t64_1 >= *(int64_t *)&t64_2);
            assert(result);
            break;
        case BC_GE_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, t64_1 >= t64_2);
            assert(result);
            break;
        case BC_GE_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(float *)&t32_1 >= *(float *)&t32_2);
            assert(result);
            break;
        case BC_GE_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(double *)&t64_1 >= *(double *)&t64_2);
            assert(result);
            break;

        case BC_LE_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(int32_t *)&t32_1 <= *(int32_t *)&t32_2);
            assert(result);
            break;
        case BC_LE_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, t32_1 <= t32_2);
            assert(result);
            break;
        case BC_LE_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(int64_t *)&t64_1 <= *(int64_t *)&t64_2);
            assert(result);
            break;
        case BC_LE_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, t64_1 <= t64_2);
            assert(result);
            break;
        case BC_LE_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, *(float *)&t32_1 <= *(float *)&t32_2);
            assert(result);
            break;
        case BC_LE_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush32(vm, *(double *)&t64_1 <= *(double *)&t64_2);
            assert(result);
            break;

        case BC_NEG_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, negate32(t32_1));
            assert(result);
            break;
        case BC_NEG_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, negate64(t64_1));
            assert(result);
            break;
        case BC_NEG_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, negateF32(t32_1));
            assert(result);
            break;
        case BC_NEG_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, negateF64(t64_1));
            assert(result);
            break;

        case BC_NOT_32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, ~t32_1);
            assert(result);
            break;
        case BC_NOT_64:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, ~t32_1);
            assert(result);
            break;

        case BC_ADD_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, add32(t32_1, t32_2));
            assert(result);
            break;
        case BC_ADD_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, add64(t64_1, t64_2));
            assert(result);
            break;
        case BC_ADD_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, addF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_ADD_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, addF64(t64_1, t64_2));
            assert(result);
            break;

        case BC_SUB_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, sub32(t32_1, t32_2));
            assert(result);
            break;
        case BC_SUB_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, sub64(t64_1, t64_2));
            assert(result);
            break;
        case BC_SUB_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, subF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_SUB_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, subF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_MUL_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, mul32(t32_1, t32_2));
            assert(result);
            break;
        case BC_MUL_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, mul64(t64_1, t64_2));
            assert(result);
            break;
        case BC_MUL_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, mulF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_MUL_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, mulF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_DIV_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, divU32(t32_1, t32_2));
            assert(result);
            break;
        case BC_DIV_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, divU64(t64_1, t64_2));
            assert(result);
            break;
        case BC_DIV_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, divI32(t32_1, t32_2));
            assert(result);
            break;
        case BC_DIV_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, divI64(t64_1, t64_2));
            assert(result);
            break;
        case BC_DIV_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, divF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_DIV_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, divF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_REM_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, remI32(t32_1, t32_2));
            assert(result);
            break;
        case BC_REM_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, remI64(t64_1, t64_2));
            assert(result);
            break;
        case BC_REM_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, remU32(t32_1, t32_2));
            assert(result);
            break;
        case BC_REM_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, remU64(t64_1, t64_2));
            assert(result);
            break;
        case BC_REM_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, remF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_REM_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, remF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_EXTEND_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, extendI32(t32_1));
            assert(result);
            break;
        case BC_EXTEND_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, extendU32(t32_1));
            assert(result);
            break;
        case BC_WRAP_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush32(vm, *(uint32_t *)&t64_1);
            assert(result);
            break;
        case BC_PROMOTE_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, promoteF32(t32_1));
            assert(result);
            break;
        case BC_DEMOTE_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush32(vm, demoteF64(t64_1));
            assert(result);
            break;
        case BC_CONVERT_F32_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, convertI32ToF32(t32_1));
            assert(result);
            break;
        case BC_CONVERT_F32_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush32(vm, convertI64ToF32(t64_1));
            assert(result);
            break;
        case BC_CONVERT_F32_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, convertU32ToF32(t32_1));
            assert(result);
            break;
        case BC_CONVERT_F32_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush32(vm, convertU64ToF32(t64_1));
            assert(result);
            break;
        case BC_CONVERT_F64_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, convertI32ToF64(t32_1));
            assert(result);
            break;
        case BC_CONVERT_F64_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, convertI64ToF64(t64_1));
            assert(result);
            break;
        case BC_CONVERT_F64_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, convertU32ToF64(t32_1));
            assert(result);
            break;
        case BC_CONVERT_F64_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, convertU64ToF64(t64_1));
            assert(result);
            break;
        case BC_CONVERT_I32_STR: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            result = ntPush32(vm, ntStringToI32(str));
            assert(result);
            break;
        }
        case BC_CONVERT_U32_STR: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            result = ntPush32(vm, ntStringToU32(str));
            assert(result);
            break;
        }
        case BC_CONVERT_I64_STR: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            result = ntPush64(vm, ntStringToI64(str));
            assert(result);
            break;
        }
        case BC_CONVERT_U64_STR: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            result = ntPush64(vm, ntStringToU64(str));
            assert(result);
            break;
        }
        case BC_CONVERT_F32_STR: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            result = ntPush32(vm, ntStringToF32(str));
            assert(result);
            break;
        }
        case BC_CONVERT_F64_STR: {
            const NT_STRING *str;
            result = ntPopObject(vm, (NT_OBJECT **)&str);
            assert(result);
            result = ntPush64(vm, ntStringToF64(str));
            assert(result);
            break;
        }
        case BC_CONVERT_STR_I32: {
            result = ntPop32(vm, &t32_1);
            assert(result);

            const NT_TYPE *const type = ntI32Type();
            const NT_STRING *const str = type->string((NT_OBJECT *)&t32_1);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_CONVERT_STR_U32: {
            result = ntPop32(vm, &t32_1);
            assert(result);

            const NT_TYPE *const type = ntU32Type();
            const NT_STRING *const str = type->string((NT_OBJECT *)&t32_1);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_CONVERT_STR_I64: {
            result = ntPop64(vm, &t64_1);
            assert(result);

            const NT_TYPE *const type = ntI64Type();
            const NT_STRING *const str = type->string((NT_OBJECT *)&t64_1);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_CONVERT_STR_U64: {
            result = ntPop64(vm, &t64_1);
            assert(result);

            const NT_TYPE *const type = ntU64Type();
            const NT_STRING *const str = type->string((NT_OBJECT *)&t64_1);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_CONVERT_STR_F32: {
            result = ntPop32(vm, &t32_1);
            assert(result);

            const NT_TYPE *const type = ntF32Type();
            const NT_STRING *const str = type->string((NT_OBJECT *)&t32_1);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_CONVERT_STR_F64: {
            result = ntPop64(vm, &t64_1);
            assert(result);

            const NT_TYPE *const type = ntF64Type();
            const NT_STRING *const str = type->string((NT_OBJECT *)&t64_1);
            result = ntPushObject(vm, (NT_OBJECT *)str);
            assert(result);
            break;
        }
        case BC_TRUNCATE_I32_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, truncateFloatToI32(t32_1));
            assert(result);
            break;
        case BC_TRUNCATE_I64_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, truncateFloatToI64(t32_1));
            assert(result);
            break;
        case BC_TRUNCATE_U32_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, truncateFloatToU32(t32_1));
            assert(result);
            break;
        case BC_TRUNCATE_U64_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush64(vm, truncateFloatToU64(t32_1));
            assert(result);
            break;
        case BC_TRUNCATE_I32_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush32(vm, truncateDoubleToI32(t64_1));
            assert(result);
            break;
        case BC_TRUNCATE_I64_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, truncateDoubleToI64(t64_1));
            assert(result);
            break;
        case BC_TRUNCATE_U32_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush32(vm, truncateDoubleToU32(t64_1));
            assert(result);
            break;
        case BC_TRUNCATE_U64_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, truncateDoubleToU64(t64_1));
            assert(result);
            break;
        case BC_MIN_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, minF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_MIN_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, minF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_MAX_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, maxF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_MAX_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, maxF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_NEAREST_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, nearestF32(t32_1));
            assert(result);
            break;
        case BC_NEAREST_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, nearestF64(t64_1));
            assert(result);
            break;
        case BC_CEIL_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, ceilF32(t32_1));
            assert(result);
            break;
        case BC_CEIL_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, ceilF64(t64_1));
            assert(result);
            break;
        case BC_FLOOR_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, floorF32(t32_1));
            assert(result);
            break;
        case BC_FLOOR_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, floorF64(t64_1));
            assert(result);
            break;
        case BC_TRUNCATE_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, truncateF32(t32_1));
            assert(result);
            break;
        case BC_TRUNCATE_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, truncateF64(t64_1));
            assert(result);
            break;
        case BC_ABS_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, absF32(t32_1));
            assert(result);
            break;
        case BC_ABS_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, absF64(t64_1));
            assert(result);
            break;
        case BC_SQRT_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, sqrtF32(t32_1));
            assert(result);
            break;
        case BC_SQRT_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, sqrtF64(t64_1));
            assert(result);
            break;
        case BC_COPYSIGN_F32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, copysignF32(t32_1, t32_2));
            assert(result);
            break;
        case BC_COPYSIGN_F64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, copysignF64(t64_1, t64_2));
            assert(result);
            break;
        case BC_AND_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, and32(t32_1, t32_2));
            assert(result);
            break;
        case BC_AND_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, and64(t64_1, t64_2));
            assert(result);
            break;
        case BC_OR_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, or32(t32_1, t32_2));
            assert(result);
            break;
        case BC_OR_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, or64(t64_1, t64_2));
            assert(result);
            break;
        case BC_XOR_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, xor32(t32_1, t32_2));
            assert(result);
            break;
        case BC_XOR_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, xor64(t64_1, t64_2));
            assert(result);
            break;
        case BC_SHL_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, shl32(t32_1, t32_2));
            assert(result);
            break;
        case BC_SHL_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, shl64(t64_1, t64_2));
            assert(result);
            break;
        case BC_SHR_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, shrS32(t32_1, t32_2));
            assert(result);
            break;
        case BC_SHR_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, shrS64(t64_1, t64_2));
            assert(result);
            break;
        case BC_SHR_U32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, shrU32(t32_1, t32_2));
            assert(result);
            break;
        case BC_SHR_U64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, shrU64(t64_1, t64_2));
            assert(result);
            break;
        case BC_ROL_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, rol32(t32_1, t32_2));
            assert(result);
            break;
        case BC_ROL_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, rol64(t64_1, t64_2));
            assert(result);
            break;
        case BC_ROR_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPop32(vm, &t32_2);
            assert(result);
            result = ntPush32(vm, ror32(t32_1, t32_2));
            assert(result);
            break;
        case BC_ROR_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPop64(vm, &t64_2);
            assert(result);
            result = ntPush64(vm, ror64(t64_1, t64_2));
            assert(result);
            break;
        case BC_CLZ_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, clz32(t32_1));
            assert(result);
            break;
        case BC_CLZ_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, clz64(t64_1));
            assert(result);
            break;
        case BC_CTZ_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, ctz32(t32_1));
            assert(result);
            break;
        case BC_CTZ_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, ctz64(t64_1));
            assert(result);
            break;
        case BC_POP:
            vm->pc += ntReadVariant(vm->chunk, vm->pc, &t64_1);
            t64_2 = 0;
            if (t64_1 > 0)
                do
                {
                    ntPop32(vm, &t32_1);
                    t64_2++;
                } while (t64_2 != t64_1);
            break;
        case BC_POP_32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            break;
        case BC_POP_64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            break;
        case BC_POPCNT_I32:
            result = ntPop32(vm, &t32_1);
            assert(result);
            result = ntPush32(vm, popcount32(t32_1));
            assert(result);
            break;
        case BC_POPCNT_I64:
            result = ntPop64(vm, &t64_1);
            assert(result);
            result = ntPush64(vm, popcount64(t64_1));
            assert(result);
            break;

        case BC_RETURN:
            return NT_OK;
        }

        if (vm->stackOverflow)
            return NT_STACK_OVERFLOW;
    }
}

NT_RESULT ntRun(NT_VM *vm, NT_CHUNK *chunk)
{
    vm->chunk = chunk;
    vm->pc = 0;
    return run(vm);
}
