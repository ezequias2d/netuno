#ifndef NETUNO_OBJECT_H
#define NETUNO_OBJECT_H

#include "array.h"
#include "common.h"

#define IS_TYPE(obj, type) ((obj)->type == (type))

typedef enum
{
    NT_OBJECT_ERROR,
    NT_OBJECT_STRING,
    NT_OBJECT_F64,
    NT_OBJECT_F32,
    NT_OBJECT_U64,
    NT_OBJECT_I64,
    NT_OBJECT_U32,
    NT_OBJECT_I32,
    NT_OBJECT_FUNCTION,
    NT_OBJECT_CUSTOM,
} NT_OBJECT_TYPE;

typedef struct _NT_VM NT_VM;
typedef struct _NT_TYPE NT_TYPE;
typedef struct _NT_DELEGATE_TYPE NT_DELEGATE_TYPE;
typedef struct _NT_PARAM NT_PARAM;

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_STRING NT_STRING;
typedef struct _NT_FUNCTION NT_FUNCTION;

typedef void (*freeObj)(NT_OBJECT *obj);
typedef const NT_STRING *(*toString)(NT_OBJECT *obj);
typedef bool (*equalsObj)(NT_OBJECT *obj1, NT_OBJECT *obj2);
typedef const NT_STRING *(*concatObj)(NT_OBJECT *obj1, NT_OBJECT *obj2);
typedef void (*nativeFun)(NT_VM *vm, const NT_TYPE *delegateType);

struct _NT_TYPE
{
    NT_OBJECT_TYPE objectType;
    const NT_STRING *typeName;
    freeObj free;
    toString string;
    equalsObj equals;
    size_t stackSize;
    size_t instanceSize;
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

struct _NT_STRING
{
    NT_OBJECT object;
    size_t length;
    char_t *chars;
    uint32_t hash;
};

const NT_TYPE *ntStringType(void);
const NT_STRING *ntCopyString(const char_t *chars, const size_t length);
const NT_STRING *ntTakeString(char_t *chars, const size_t length);
bool ntStrEquals(const char_t *str1, const char_t *str2);
bool ntStrEqualsFixed(const char_t *str1, const size_t size1, const char_t *str2,
                      const size_t size2);

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

const NT_TYPE *ntI32Type(void);
const NT_TYPE *ntI64Type(void);
const NT_TYPE *ntU32Type(void);
const NT_TYPE *ntU64Type(void);
const NT_TYPE *ntF32Type(void);
const NT_TYPE *ntF64Type(void);

const NT_TYPE *ntErrorType(void);

#endif
