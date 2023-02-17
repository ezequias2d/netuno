#ifndef NT_NATIVE_H
#define NT_NATIVE_H

#include "netuno/symbol.h"
#include <netuno/object.h>
#include <netuno/vm.h>

const NT_DELEGATE_TYPE *ntCreateNativeFunction(NT_MODULE *module, const char_t *name,
                                               const NT_TYPE *returnType, size_t paramCount,
                                               const NT_PARAM *params, nativeFun func, bool public);

#endif
