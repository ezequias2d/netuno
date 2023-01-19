#ifndef NT_CUSTOM_TYPE_H
#define NT_CUSTOM_TYPE_H

#include <netuno/function.h>
#include <netuno/type.h>

typedef struct _NT_CUSTOM_TYPE NT_CUSTOM_TYPE;
typedef struct _NT_FIELD NT_FIELD;

struct _NT_CUSTOM_TYPE
{
    NT_TYPE type;
    const NT_FUNCTION *free;
    const NT_FUNCTION *string;
    const NT_FUNCTION *equals;
    NT_TABLE fields;
};

struct _NT_FIELD
{
    const NT_TYPE *fieldType;
    size_t offset;
};

#endif
