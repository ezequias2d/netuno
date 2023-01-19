#ifndef NT_FUNCTION_H
#define NT_FUNCTION_H

#include <netuno/object.h>
#include <netuno/type.h>

typedef struct _NT_PARAM NT_PARAM;
typedef struct _NT_DELEGATE_TYPE NT_DELEGATE_TYPE;

// --- DELEGATE ---
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
const NT_DELEGATE_TYPE *ntCreateDelegateType(const NT_STRING *name, const NT_TYPE *returnType,
                                             size_t paramCount, const NT_PARAM *params);

// --- FUNCTION ---
struct _NT_FUNCTION
{
    NT_OBJECT object;
    const NT_STRING *name;
    union {
        struct
        {
            size_t addr;
            const NT_CHUNK *sourceChunk;
        };
        nativeFun func;
    };
    bool native;
};

#endif
