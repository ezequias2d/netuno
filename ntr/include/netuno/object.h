#ifndef NT_OBJECT_H
#define NT_OBJECT_H

#include <netuno/array.h>
#include <netuno/common.h>
#include <netuno/symbol.h>
#include <netuno/table.h>

#define IS_VALID_OBJECT(obj) ((obj) && IS_VALID_TYPE(((NT_OBJECT *)(obj))->type))

typedef struct _NT_VM NT_VM;
typedef struct _NT_MODULE NT_MODULE;

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_TYPE NT_TYPE;
typedef struct _NT_DELEGATE_TYPE NT_DELEGATE_TYPE;
typedef struct _NT_STRING NT_STRING;

typedef bool (*nativeFun)(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType);

struct _NT_OBJECT
{
    const NT_TYPE *type;
    size_t refCount;
};

const NT_TYPE *ntObjectType(void);
NT_OBJECT *ntCreateObject(const NT_TYPE *type);
void ntRefObject(NT_OBJECT *object);
void ntFreeObject(NT_OBJECT *object);
void ntMakeConstant(NT_OBJECT *object);
void ntForceFreeObject(NT_OBJECT *object);
const NT_STRING *ntToString(NT_OBJECT *object);
bool ntEquals(NT_OBJECT *object1, NT_OBJECT *object2);
const NT_STRING *ntConcat(NT_OBJECT *object1, NT_OBJECT *object2);

const NT_TYPE *ntBoolType(void);
const NT_TYPE *ntI32Type(void);
const NT_TYPE *ntI64Type(void);
const NT_TYPE *ntU32Type(void);
const NT_TYPE *ntU64Type(void);
const NT_TYPE *ntF32Type(void);
const NT_TYPE *ntF64Type(void);

const NT_TYPE *ntUndefinedType(void);
const NT_TYPE *ntVoidType(void);
const NT_TYPE *ntErrorType(void);

#endif
