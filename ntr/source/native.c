#include <netuno/module.h>
#include <netuno/native.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/symbol.h>

const NT_DELEGATE_TYPE *ntCreateNativeFunction(NT_MODULE *module, const char_t *name,
                                               const NT_TYPE *returnType, size_t paramCount,
                                               const NT_PARAM *params, nativeFun func, bool public)
{
    char_t *delegateTypeNameCstr = ntDelegateTypeName(NULL, paramCount, params);
    const NT_STRING *delegateTypeName =
        ntTakeString(delegateTypeNameCstr, ntStrLen(delegateTypeNameCstr));

    const NT_DELEGATE_TYPE *const delegateType =
        ntCreateDelegateType(delegateTypeName, returnType, paramCount, params);

    const NT_STRING *functionName = ntCopyString(name, ntStrLen(name));
    ntAddNativeModuleFunction(module, functionName, delegateType, func, public);

    return delegateType;
}
