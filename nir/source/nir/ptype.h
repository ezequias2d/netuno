#ifndef NIR_PTYPE_H
#define NIR_PTYPE_H

#include "nir/plist.h"
#include <netuno/nir/type.h>

struct _NIR_TYPE
{
    NIR_TYPE_ID id;
    NIR_CONTEXT *context;
};

struct _NIR_ARRAY_TYPE
{
    NIR_TYPE type;           // Base type structure.
    NIR_TYPE *containedType; // The element type of the array.
    uint64_t numElements;    // Number of elements in the array.
};
typedef struct _NIR_ARRAY_TYPE NIR_ARRAY_TYPE;

struct _NIR_FUNCTION_TYPE
{
    NIR_TYPE type; // Base type structure.
    NIR_TYPE *result;
    size_t paramCount;
    NIR_TYPE **params;
    bool isVarArg;
};
typedef struct _NIR_FUNCTION_TYPE NIR_FUNCTION_TYPE;

struct _NIR_INTEGER_TYPE
{
    NIR_TYPE type;            // Base type structure.
    NIR_INTEGER_BITS numBits; // Number of bits;
};
typedef struct _NIR_INTEGER_TYPE NIR_INTEGER_TYPE;

struct _NIR_POINTER_TYPE
{
    NIR_TYPE type; // Base type structure.
    NIR_TYPE *pointeeType;
};
typedef struct _NIR_POINTER_TYPE NIR_POINTER_TYPE;

struct _NIR_STRUCT_TYPE
{
    NIR_TYPE type; // Base type structure.
    bool hasBody;
    bool isSized;
    // NT_STRING *name; // Struct name.

    size_t elementCount;
    NIR_TYPE **elements;
};
typedef struct _NIR_STRUCT_TYPE NIR_STRUCT_TYPE;

#endif
