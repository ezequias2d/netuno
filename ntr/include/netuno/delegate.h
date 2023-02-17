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
#ifndef NT_DELEGATE_H
#define NT_DELEGATE_H

#include <netuno/object.h>
#include <netuno/type.h>

typedef struct _NT_PARAM NT_PARAM;
typedef struct _NT_DELEGATE NT_DELEGATE;
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
    NT_PARAM *params;
};

struct _NT_DELEGATE
{
    NT_OBJECT object;
    const NT_STRING *name;
    union {
        struct
        {
            size_t addr;
            const NT_MODULE *sourceModule;
        };
        nativeFun func;
    };
    bool native;
};

const NT_TYPE *ntDelegateType(void);
char_t *ntDelegateTypeName(const NT_TYPE *returnType, size_t paramCount, const NT_PARAM *params);
const NT_DELEGATE_TYPE *ntCreateDelegateType(const NT_STRING *delegateTypeName,
                                             const NT_TYPE *returnType, size_t paramCount,
                                             const NT_PARAM *params);
const NT_DELEGATE *ntDelegate(const NT_DELEGATE_TYPE *delegateType, const NT_MODULE *module,
                              size_t addr, const NT_STRING *name);
const NT_DELEGATE *ntNativeDelegate(const NT_DELEGATE_TYPE *delegateType, nativeFun func,
                                    const NT_STRING *name);

#endif
