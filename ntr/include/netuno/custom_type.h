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
