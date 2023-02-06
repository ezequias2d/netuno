#ifndef NETUNO_TYPE_H
#define NETUNO_TYPE_H

#include "netuno/symbol.h"
#include <netuno/common.h>
#include <netuno/object.h>

#define IS_TYPE(obj, type) (((NT_OBJECT *)(obj))->type == (type))
#define IS_VALID_TYPE(type)                                                                        \
    ((type) && ((NT_TYPE *)(type))->objectType >= NT_OBJECT_ERROR &&                               \
     ((NT_TYPE *)(type))->objectType <= NT_OBJECT_TYPE_MAX)

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
    NT_OBJECT_DELEGATE,
    NT_OBJECT_ASSEMBLY,
    NT_OBJECT_MODULE,
    NT_OBJECT_OBJECT,
    NT_OBJECT_TYPE_TYPE,
    NT_OBJECT_CUSTOM,

    NT_OBJECT_TYPE_MIN = NT_OBJECT_ERROR,
    NT_OBJECT_TYPE_MAX = NT_OBJECT_CUSTOM,
} NT_OBJECT_TYPE;

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_STRING NT_STRING;
typedef struct _NT_TYPE NT_TYPE;

typedef void (*freeObj)(NT_OBJECT *obj);
typedef const NT_STRING *(*toString)(NT_OBJECT *obj);
typedef bool (*equalsObj)(NT_OBJECT *obj1, NT_OBJECT *obj2);

struct _NT_TYPE
{
    NT_OBJECT object;
    NT_OBJECT_TYPE objectType;
    const NT_STRING *typeName;
    freeObj free;
    toString string;
    equalsObj equals;
    size_t stackSize;
    size_t instanceSize;
    const NT_TYPE *baseType;
    NT_SYMBOL_TABLE fields;
};

const NT_TYPE *ntType(void);
bool ntTypeIsAssignableFrom(const NT_TYPE *to, const NT_TYPE *from);

#endif
