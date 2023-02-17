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
#include <assert.h>
#include <netuno/console.h>
#include <netuno/memory.h>
#include <netuno/native.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/vm.h>
#include <stdio.h>

static NT_MODULE CONSOLE = {
    .type.object =
        {
            .type = NULL,
        },
};

static const NT_DELEGATE_TYPE *ConsoleWriteType = NULL;
static bool consoleWrite(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType)
{
    assert(vm);
    assert(delegateType);
    assert(delegateType == ConsoleWriteType);

    NT_OBJECT *object;
    bool result = ntPopRef(vm, (NT_REF *)&object);
    assert(result);
    assert(IS_VALID_OBJECT(object));
    assert(ntTypeIsAssignableFrom(ntObjectType(), object->type));

    const NT_STRING *str = ntToString(object);
    char *s = ntToCharFixed(str->chars, str->length);
    ntFreeObject((NT_OBJECT *)str);

    printf("%s", s);
    ntFree(s);

    return true;
}

static void addWrite(void)
{
    NT_PARAM param = {
        .type = ntObjectType(),
        .name = ntCopyString(U"object", 6),
    };

    ConsoleWriteType =
        ntCreateNativeFunction(&CONSOLE, U"write", NULL, 1, &param, consoleWrite, true);
}

static const NT_DELEGATE_TYPE *ConsoleReadLineType = NULL;
static bool consoleReadline(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType)
{
    assert(vm);
    assert(delegateType);
    assert(delegateType == ConsoleReadLineType);

    size_t lenmax = 256, len = lenmax;
    char *line = ntMalloc(lenmax), *linep = line;
    int c;

    if (line == NULL)
        return false;

    for (;;)
    {
        c = fgetc(stdin);
        if (c == EOF)
            break;

        if (--len == 0)
        {
            len = lenmax;
            char *linen = ntRealloc(linep, lenmax *= 2);

            if (linen == NULL)
            {
                ntFree(linep);
                return false;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if (c == '\n')
            break;

        *line++ = c;
    }
    *line = '\0';

    char_t *utf32 = ntToCharT(linep);
    ntFree(linep);

    const NT_STRING *str = ntTakeString(utf32, ntStrLen(utf32));
    if (!ntPushRef(vm, (NT_REF)str))
    {
        ntFreeObject((NT_OBJECT *)str);
        return false;
    }

    return true;
}

static void addReadline(void)
{
    ConsoleReadLineType = ntCreateNativeFunction(&CONSOLE, U"readline", ntStringType(), 0, NULL,
                                                 consoleReadline, true);
}

const NT_MODULE *ntConsoleModule(void)
{
    if (CONSOLE.type.object.type == NULL)
    {
        ntInitModule(&CONSOLE);
        ntMakeConstant((NT_OBJECT *)&CONSOLE);
        const char_t *moduleName = U"console";
        CONSOLE.type.typeName = ntCopyString(moduleName, ntStrLen(moduleName));
        ntInitSymbolTable(&CONSOLE.type.fields, (NT_SYMBOL_TABLE *)&ntType()->fields, STT_TYPE,
                          NULL);

        addWrite();
        addReadline();
    }

    return &CONSOLE;
}
