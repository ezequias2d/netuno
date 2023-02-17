/*
MIT License

Copyright (c) 2022 Ezequias Silva <ezequiasmoises@gmail.com> and the Netuno
contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
