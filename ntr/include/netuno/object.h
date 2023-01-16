#ifndef NETUNO_OBJECT_H
#define NETUNO_OBJECT_H

#include <netuno/array.h>
#include <netuno/common.h>
#include <netuno/symbol.h>
#include <netuno/table.h>
#include <netuno/type.h>

typedef struct _NT_VM NT_VM;
typedef struct _NT_FIELD NT_FIELD;
typedef struct _NT_CUSTOM_TYPE NT_CUSTOM_TYPE;
typedef struct _NT_DELEGATE_TYPE NT_DELEGATE_TYPE;
typedef struct _NT_PARAM NT_PARAM;
typedef struct _NT_CHUNK NT_CHUNK;

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_STRING NT_STRING;
typedef struct _NT_FUNCTION NT_FUNCTION;

typedef void (*nativeFun)(NT_VM *vm, const NT_TYPE *delegateType);

struct _NT_FIELD
{
    const NT_TYPE *fieldType;
    size_t offset;
};

struct _NT_CUSTOM_TYPE
{
    NT_TYPE type;
    const NT_FUNCTION *free;
    const NT_FUNCTION *string;
    const NT_FUNCTION *equals;
    NT_TABLE fields;
};

struct _NT_OBJECT
{
    const NT_TYPE *type;
    size_t refCount;
};

NT_OBJECT *ntCreateObject(const NT_TYPE *type);
void ntFreeObject(NT_OBJECT *object);
const NT_STRING *ntToString(NT_OBJECT *object);
bool ntEquals(NT_OBJECT *object1, NT_OBJECT *object2);
const NT_STRING *ntConcat(NT_OBJECT *object1, NT_OBJECT *object2);



struct _NT_FUNCTION
{
    NT_OBJECT object;
    NT_STRING *name;
    union {
        struct
        {
            size_t addr;
            // NT_CHUNK *sourceChunk;
        };
        nativeFun func;
    };
    bool native;
};

struct _NT_PARAM
{
    const NT_TYPE *type;
    const NT_STRING *name;
};

struct _NT_DELEGATE_TYPE
{
    NT_TYPE type;
    size_t paramCount;
    const NT_TYPE *returnType;
    NT_PARAM params[];
};

char_t *ntDelegateTypeName(const NT_TYPE *returnType, size_t paramCount, const NT_PARAM *params);
NT_TYPE *ntCreateDelegateType(const NT_STRING *name, const NT_TYPE *returnType, size_t paramCount,
                              const NT_PARAM *params);

const NT_TYPE *ntBoolType(void);
const NT_TYPE *ntI32Type(void);
const NT_TYPE *ntI64Type(void);
const NT_TYPE *ntU32Type(void);
const NT_TYPE *ntU64Type(void);
const NT_TYPE *ntF32Type(void);
const NT_TYPE *ntF64Type(void);

const NT_TYPE *ntErrorType(void);

#endif
