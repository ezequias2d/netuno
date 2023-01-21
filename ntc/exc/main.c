#include <netuno/common.h>
#include <netuno/debug.h>
#include <netuno/delegate.h>
#include <netuno/memory.h>
#include <netuno/ntc.h>
#include <netuno/nto.h>
#include <netuno/str.h>
#include <netuno/vm.h>
#include <stdint.h>
#include <stdio.h>

const char *readFile(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *code = (char *)ntMalloc(fsize + 1);
    fread(code, fsize, 1, file);
    fclose(file);

    code[fsize] = '\0';
    return code;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Error: need a file to execute");
        return 2;
    }

    const char *filename = argv[1];
    const char *code = readFile(filename);
    if (code == NULL)
    {
        printf("Error: could not open file %s\n", filename);
        return 1;
    }

    const char_t *codet = ntToCharT(code);
    ntFree((void *)code);

    NT_ASSEMBLY *assembly = ntCreateAssembly();

    const NT_DELEGATE *entryPoint = NULL;
    ntCompile(assembly, codet, U"main", &entryPoint);

    ntFree((void *)codet);

    NT_VM *vm = ntCreateVM();
    ntRun(vm, entryPoint);

    uint32_t result = INT32_MAX;
    if (!ntPop32(vm, &result))
    {
        printf("Error: No return value in main!\n");
    }

    ntFreeVM(vm);
    ntFreeAssembly(assembly);

    return result;
}
