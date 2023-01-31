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

static const char *readFile(const char *filepath)
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

        if (object->type->objectType != NT_OBJECT_MODULE)
            continue;

        NT_MODULE *const module = (NT_MODULE *)object;
        NT_SYMBOL_ENTRY entry;
        if (!ntLookupSymbolCurrent(&module->fields, entryPoint, ntStrLen(entryPoint), &entry))
            continue;

        if ((entry.type & SYMBOL_TYPE_FUNCTION) == SYMBOL_TYPE_FUNCTION ||
            (entry.type & SYMBOL_TYPE_SUBROUTINE) == SYMBOL_TYPE_SUBROUTINE)
            return (const NT_DELEGATE *)entry.data;
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Error: need a file to execute");
        return 2;
    }

    const char *filepath = argv[1];
    const char *code = readFile(filepath);
    if (code == NULL)
    {
        printf("Error: could not open file %s\n", filepath);
        return 1;
    }

    const char_t *filename;
    {
        const char_t *filepatht = ntToCharT(filepath);
        const char_t *filenamet = ntStrRChr(filepatht, U'/') + 1;
        const char_t *dott = ntStrChr(filenamet, U'.');
        const size_t lent = dott - filenamet;

        char_t *fn = (char_t *)ntMalloc(sizeof(char_t) * (lent + 1));
        ntMemcpy(fn, filenamet, lent * sizeof(char_t));
        ntFree((char_t *)filepatht);

        fn[lent] = U'\0';
        filename = fn;
    }

    const char_t *codet = ntToCharT(code);
    ntFree((void *)code);
    NT_ASSEMBLY *assembly = ntCreateAssembly();
    ntCompile(assembly, codet, filename);
    ntFree((void *)codet);

    NT_VM *vm = ntCreateVM();

    const NT_DELEGATE *entryPoint = findEntryPoint(assembly, U"main");
    if (entryPoint == NULL)
    {
        printf("Error: No entry point main!");
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
