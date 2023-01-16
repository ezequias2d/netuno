#ifndef NETUNO_NTO_H
#define NETUNO_NTO_H

#include <netuno/array.h>
#include <netuno/object.h>
#include <netuno/table.h>

typedef enum
{
#define bytecode(a) BC_##a,
#include "opcode.inc"
#undef bytecode
    BC_LAST,
} NT_OPCODE;

typedef struct _NT_LOCAL_TYPE
{
    NT_OBJECT_TYPE objectType;
    uint64_t constantTypeName;
    uint64_t stackSize;
    uint64_t instanceSize;
} NT_LOCAL_TYPE;

typedef struct _NT_LOCAL_FIELD
{
    uint64_t constantType;
    uint64_t constantName;
    uint64_t fieldOffset;
} NT_LOCAL_FIELD;

typedef struct _NT_LOCAL_CUSTOM_TYPE
{
    NT_LOCAL_TYPE localType;
    uint64_t constantFreeFunction;
    uint64_t constantStringFunction;
    uint64_t constantEqualsFunction;
    uint64_t fieldsCount;
    NT_LOCAL_FIELD fields[];
} NT_LOCAL_CUSTOM_TYPE;

typedef struct _NT_LOCAL_PARAM
{
    uint64_t constantType;
    uint64_t constantName;
} NT_LOCAL_PARAM;

typedef struct _NT_LOCAL_DELEGATE_TYPE
{
    uint64_t constantReturnType;
    uint64_t paramCount;
    NT_LOCAL_PARAM constantParams[];
} NT_LOCAL_DELEGATE_TYPE;

typedef struct _NT_LOCAL_FUNCTION
{
    uint64_t addr;
    uint64_t constantDelegateType;
    uint64_t constantString;
} NT_LOCAL_FUNCTION;

typedef struct _NT_CHUNK
{
    NT_ARRAY code;
    NT_ARRAY lines;
    NT_ARRAY constants;
} NT_CHUNK;

typedef struct
{
    size_t start;
    size_t line;
} NT_LINE;

typedef struct _NT_ASSEMBLY
{
    NT_ARRAY chunks;
    NT_ARRAY types;
    NT_TABLE table;
    NT_TABLE delegates;
    NT_TABLE strings;
} NT_ASSEMBLY;

NT_ASSEMBLY *ntCreateAssembly(void);
void ntFreeAssembly(NT_ASSEMBLY *assembly);
const NT_TYPE *ntDelegateType(NT_ASSEMBLY *assembly, const NT_TYPE *returnType, size_t count,
                              const NT_PARAM *params);

NT_CHUNK *ntCreateChunk(void);
void ntInitChunk(NT_CHUNK *chunk);
void ntDeinitChunk(NT_CHUNK *chunk);
void ntFreeChunk(NT_CHUNK *chunk);
size_t ntWriteChunk(NT_CHUNK *chunk, const uint8_t value, const int64_t line);
void ntInsertChunk(NT_CHUNK *chunk, const size_t offset, const void *data, const size_t length);
void ntInsertChunkVarint(NT_CHUNK *chunk, const size_t offset, const uint64_t value);
size_t ntWriteChunkVarint(NT_CHUNK *chunk, const uint64_t value, const int64_t line);

uint64_t ntAddConstant32(NT_CHUNK *chunk, const uint32_t value);
uint64_t ntAddConstant64(NT_CHUNK *chunk, const uint64_t value);
uint64_t ntAddConstantString(NT_CHUNK *chunk, const char_t *str, const size_t length);
uint64_t ntAddConstantType(NT_CHUNK *chunk, const NT_TYPE *type);
uint64_t ntAddConstantFunction(NT_CHUNK *chunk, size_t addr, const NT_DELEGATE_TYPE *delegateType,
                               const NT_STRING *name);

uint8_t ntRead(const NT_CHUNK *chunk, const size_t offset);
size_t ntReadVariant(const NT_CHUNK *chunk, const size_t offset, uint64_t *value);
int64_t ntGetLine(const NT_CHUNK *chunk, const size_t offset, bool *atStart);

#endif
