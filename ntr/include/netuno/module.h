#ifndef NT_MODULE_H
#define NT_MODULE_H

#include <netuno/assembly.h>
#include <netuno/delegate.h>
#include <netuno/object.h>
#include <netuno/type.h>

typedef struct _NT_LINE
{
    size_t start;
    size_t line;
} NT_LINE;

typedef struct _NT_MODULE
{
    NT_TYPE type;
    NT_ARRAY code;
    NT_ARRAY lines;
    NT_ARRAY constants;
} NT_MODULE;

const NT_TYPE *ntModuleType(void);

NT_MODULE *ntCreateModule(void);
void ntInitModule(NT_MODULE *module);
size_t ntWriteModule(NT_MODULE *module, const uint8_t value, const int64_t line);
void ntInsertModule(NT_MODULE *module, const size_t offset, const void *data, const size_t length);
void ntInsertModuleVarint(NT_MODULE *module, const size_t offset, const uint64_t value);
size_t ntWriteModuleVarint(NT_MODULE *module, const uint64_t value, const int64_t line);

void ntAddModuleWeakFunction(NT_MODULE *module, const NT_STRING *name,
                             const NT_DELEGATE_TYPE *delegateType, bool public);
const NT_DELEGATE *ntAddModuleFunction(NT_MODULE *module, const NT_STRING *name,
                                       const NT_DELEGATE_TYPE *delegateType, size_t pc,
                                       bool public);
const NT_DELEGATE *ntAddNativeModuleFunction(NT_MODULE *module, const NT_STRING *name,
                                             const NT_DELEGATE_TYPE *delegateType, nativeFun func,
                                             bool public);

uint64_t ntAddConstant32(NT_MODULE *module, const uint32_t value);
uint64_t ntAddConstant64(NT_MODULE *module, const uint64_t value);

uint8_t ntRead(const NT_MODULE *module, const size_t offset);
size_t ntReadVariant(const NT_MODULE *module, const size_t offset, uint64_t *value);
int64_t ntGetLine(const NT_MODULE *module, const size_t offset, bool *atStart);
#endif
