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
#include <netuno/common.h>
#include <netuno/debug.h>
#include <netuno/delegate.h>
#include <netuno/memory.h>
#include <netuno/ntc.h>
#include <netuno/str.h>
#include <netuno/vm.h>
#include <stdint.h>
#include <stdio.h>

static char *readFile(const char *filepath, size_t *length)
{
    FILE *file = fopen(filepath, "r");
    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *code = (char *)ntMalloc(fsize + 1);
    fread(code, fsize, 1, file);
    fclose(file);

    code[fsize] = '\0';
    *length = fsize;
    return code;
}

static const NT_DELEGATE *findEntryPoint(const NT_ASSEMBLY *assembly, char_t *entryPoint)
{
    for (size_t i = 0; i < assembly->objects->count / sizeof(NT_REF); ++i)
    {
        NT_OBJECT *object = NULL;
        const bool result = ntArrayGet(assembly->objects, i * sizeof(NT_REF), &object,
                                       sizeof(NT_REF)) == sizeof(NT_REF);
        assert(result);

        assert(object);
        assert(IS_VALID_OBJECT(object));

        if (object->type->objectType != NT_OBJECT_TYPE_TYPE ||
            ((NT_TYPE *)object)->objectType != NT_OBJECT_MODULE)
            continue;

        NT_MODULE *const module = (NT_MODULE *)object;
        NT_SYMBOL_ENTRY entry;
        if (!ntLookupSymbolCurrent(&module->type.fields, entryPoint, ntStrLen(entryPoint), &entry))
            continue;

        if ((entry.type & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION ||
            (entry.type & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE)
            return (const NT_DELEGATE *)entry.data;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Error: need a file to execute\n");
        return 2;
    }

    const size_t count = argc - 1;
    NT_FILE *files = (NT_FILE *)ntMalloc(sizeof(NT_FILE *) * count);

    for (size_t i = 0; i < count; ++i)
    {
        char_t *filepath = ntToCharT(argv[1]);
        size_t length;
        char *code = readFile(argv[1], &length);
        if (code == NULL)
        {
            printf("Error: could not open file %s\n", argv[1]);
            return 1;
        }

        char_t *codet = ntToCharTFixed(code, length);
        ntFree(code);

        files[i] = (NT_FILE){
            .code = codet,
            .source = filepath,
        };
    }

    NT_ASSEMBLY *assembly = ntCreateAssembly();
    ntCompile(assembly, count, files);

    NT_VM *vm = ntCreateVM();

    const NT_DELEGATE *entryPoint = findEntryPoint(assembly, U"main");
    if (entryPoint == NULL)
    {
        printf("Error: No entry point main!\n");
        return -1234;
    }
    ntRun(vm, assembly, entryPoint);

    uint32_t result = INT32_MAX;
    if (!ntPop32(vm, &result))
    {
        printf("Error: No return value in main!\n");
    }

    ntFreeVM(vm);
    ntFreeObject((NT_OBJECT *)assembly);

    return result;
}
