#ifndef NETUNO_TYPE_H
#define NETUNO_TYPE_H

#include <netuno/common.h>

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

typedef struct _NT_OBJECT NT_OBJECT;
typedef struct _NT_STRING NT_STRING;
typedef struct _NT_TYPE NT_TYPE;

typedef void (*freeObj)(NT_OBJECT *obj);
typedef const NT_STRING *(*toString)(NT_OBJECT *obj);
typedef bool (*equalsObj)(NT_OBJECT *obj1, NT_OBJECT *obj2);

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

#endif
