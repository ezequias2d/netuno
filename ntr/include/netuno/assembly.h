#ifndef NT_ASSEMBLY_H
#define NT_ASSEMBLY_H

#include <netuno/array.h>
#include <netuno/delegate.h>
#include <netuno/object.h>
#include <netuno/table.h>
#include <netuno/type.h>

typedef struct _NT_ASSEMBLY
{
    NT_OBJECT object;
    NT_ARRAY *objects;
} NT_ASSEMBLY;

const NT_TYPE *ntAssemblyType(void);
NT_ASSEMBLY *ntCreateAssembly(void);
const NT_DELEGATE_TYPE *ntTakeDelegateType(NT_ASSEMBLY *assembly, const NT_TYPE *returnType,
                                           size_t count, const NT_PARAM *params);
uint64_t ntAddConstantObject(NT_ASSEMBLY *assembly, NT_OBJECT *object);
NT_OBJECT *ntGetConstantObject(const NT_ASSEMBLY *assembly, uint64_t constant);

#endif
