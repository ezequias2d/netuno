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
#include "netuno/nir/module.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/ntc.h>
#include <netuno/path.h>
#include <netuno/str.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Error: need a file to execute\n");
        return 2;
    }

    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0))
    {
        const char *logo =
            "     ╗m@@@@@%╗µ                                                                 \n"
            "  ≥@ÑÑÑÑÑÑÑÑÑÑÑÑN                                                               \n"
            " åÑÑÑÑÑÑ╜²\"   ª╜%Ñµ                                                             \n"
            "åÑÑÑÑ╨            ²⌂    ▄▄▄   ▄▄⌂ ▄▄▄▄▄▄▄¡▄▄▄▄▄▄▄¿▄▄⌂   ▄▄  ▄▄▄   ▄▄   ,▄▄▄▄▄,  \n"
            "ÑÑÑÑ               █▄   ███▌  ██H ██▀▀▀▀\"▀▀▀██▀▀▀\"██∞   ██ j███▄  ██  ▄█▀▀▀▀▀██▄\n"
            "jÑM                ██▄  ██▐██ ██H ██▄▄▄▄    ▐█▌   ██∞   ██ j██▀█▄ ██ ██▀     '██\n"
            " %H              .████  ██ ▐████H ██\"\"\"     ▐█▌   ██    ██ j██ ▐█▌██ ▀█▌     ,██\n"
            "  »,           .▄████▌  ██  \"███H ██▄▄▄▄▄   ▐█▌   ▐██▄▄██▀ j██  ▐███  ▀██▄▄▄██▀ \n"
            "   ▀█▄▄▄;  µ▄▄███████.  ²²   :²². ²²²²²²²   ²²      ²?▀╙.   ²²   ²²²     \"▀▀    \n"
            "    ▀██████████████▀                                                            \n"
            "      ²▀████████▀▀                                                              \n";
        printf(
            "%s\nNatch (ntc, Netuno Compiler) 0.1.0-alpha\nCopyright (C) 2023 Ezequias Moises dos "
            "Santos Silva\n",
            logo);
        return 0;
    }

    const size_t count = argc - 1;
    NT_FILE *files = (NT_FILE *)ntMalloc(sizeof(NT_FILE) * count);

    for (size_t i = 0; i < count; ++i)
    {
        char_t *filepath = ntToCharT(argv[i + 1]);
        size_t length;
        char *code = readFile(argv[i + 1], &length);
        if (code == NULL)
        {
            printf("Error: could not open file %s\n", argv[i + 1]);
            return 1;
        }

        char_t *codet = ntToCharTFixed(code, length);
        if (!codet)
        {
            printf("Fail to covert file %s to UTF-32.\n", argv[i + 1]);
            return 2;
        }
        ntFree(code);

        files[i] = (NT_FILE){
            .code = codet,
            .source = filepath,
            .filename = ntPathFilename(filepath, false),
        };
    }

    NIR_MODULE **modules = (NIR_MODULE **)ntMalloc(sizeof(NIR_MODULE *) * count);
    if (!ntCompile(count, files, modules))
    {
        ntFree(modules);
        return -4321;
    }

    for (size_t i = 0; i < count; ++i)
        nirPrintModule(modules[i]);

    // NT_ASSEMBLY *assembly = ntCreateAssembly();
    // if (ntCompile(assembly, count, files) != assembly)
    // {
    //     ntFreeObject((NT_OBJECT *)assembly);
    //     return -4321;
    // }

    // NT_VM *vm = ntCreateVM();

    // const NT_DELEGATE *entryPoint = findEntryPoint(assembly, U"main");
    // if (entryPoint == NULL)
    // {
    //     printf("undefined reference to \"main\"\n");
    //     return -1234;
    // }
    // NT_RESULT vmResult = ntRun(vm, assembly, entryPoint);
    // if (vmResult != NT_OK)
    // {
    //     switch (vmResult)
    //     {
    //     case NT_STACK_OVERFLOW:
    //         printf("Stack Overflow!\n");
    //         break;
    //     case NT_RUNTIME_ERROR:
    //         printf("Runtime Error!\n");
    //         break;
    //     default:
    //         printf("Unknow Error Code %d\n", vmResult);
    //         break;
    //     }
    //     return INT32_MAX;
    // }

    // uint32_t result = INT32_MAX;
    // if (!ntPop32(vm, &result))
    // {
    //     printf("Error: No return value in main!\n");
    // }

    // ntFreeVM(vm);
    // ntFreeObject((NT_OBJECT *)assembly);

    // return result;
    return 0;
}
