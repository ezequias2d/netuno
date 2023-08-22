#ifndef NIL_PTYPE_H
#define NIL_PTYPE_H

#include "nil/plist.h"
#include <netuno/nil/type.h>

struct _NIL_TYPE
{
    NIL_TYPE_ID id;
    NIL_CONTEXT *context;
};

struct _NIL_ARRAY_TYPE
{
    NIL_TYPE type;           // Base type structure.
    NIL_TYPE *containedType; // The element type of the array.
    uint64_t numElements;    // Number of elements in the array.
};
typedef struct _NIL_ARRAY_TYPE NIL_ARRAY_TYPE;

struct _NIL_FUNCTION_TYPE
{
    NIL_TYPE type; // Base type structure.
    NIL_TYPE *result;
    size_t paramCount;
    NIL_TYPE **params;
    bool isVarArg;
};
typedef struct _NIL_FUNCTION_TYPE NIL_FUNCTION_TYPE;

struct _NIL_INTEGER_TYPE
{
    NIL_TYPE type;            // Base type structure.
    NIL_INTEGER_BITS numBits; // Number of bits;
};
typedef struct _NIL_INTEGER_TYPE NIL_INTEGER_TYPE;

struct _NIL_POINTER_TYPE
{
    NIL_TYPE type; // Base type structure.
    NIL_TYPE *pointeeType;
};
typedef struct _NIL_POINTER_TYPE NIL_POINTER_TYPE;

struct _NIL_STRUCT_TYPE
{
    NIL_TYPE type; // Base type structure.
    bool hasBody;
    bool isSized;
    // NT_STRING *name; // Struct name.

    size_t elementCount;
    NIL_TYPE **elements;
};
typedef struct _NIL_STRUCT_TYPE NIL_STRUCT_TYPE;

#endif
