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
