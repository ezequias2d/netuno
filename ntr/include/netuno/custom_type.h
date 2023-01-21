#ifndef NT_CUSTOM_TYPE_H
#define NT_CUSTOM_TYPE_H

#include <netuno/delegate.h>
#include <netuno/type.h>

typedef struct _NT_CUSTOM_TYPE NT_CUSTOM_TYPE;
typedef struct _NT_FIELD NT_FIELD;

struct _NT_CUSTOM_TYPE
{
    NT_TYPE type;
    const NT_DELEGATE *free;
    const NT_DELEGATE *string;
    const NT_DELEGATE *equals;
    NT_TABLE fields;
};

struct _NT_FIELD
{
    const NT_TYPE *fieldType;
    size_t offset;
};

#endif
