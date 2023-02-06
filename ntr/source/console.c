#include <assert.h>
#include <netuno/console.h>
#include <netuno/memory.h>
#include <netuno/str.h>
#include <netuno/string.h>
#include <netuno/vm.h>
#include <stdio.h>

static NT_MODULE CONSOLE = {
    .type.object =
        (NT_OBJECT){
            .type = NULL,
        },
};

static const NT_DELEGATE_TYPE *PrintType = NULL;
static bool stdPrint(NT_VM *vm, const NT_DELEGATE_TYPE *delegateType)
{
    assert(vm);
    assert(delegateType);

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

static void addPrint(void)
{
    NT_PARAM param = {
        .type = ntObjectType(),
        .name = ntCopyString(U"object", 6),
    };

    char_t *delegateTypeNameCstr = ntDelegateTypeName(NULL, 1, &param);
    const NT_STRING *delegateTypeName =
        ntTakeString(delegateTypeNameCstr, ntStrLen(delegateTypeNameCstr));

    PrintType = ntCreateDelegateType(delegateTypeName, NULL, 1, &param);
    const NT_STRING *printName = ntCopyString(U"write", 5);
    ntAddNativeModuleFunction(&CONSOLE, printName, PrintType, stdPrint, true);
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

        addPrint();
    }

    return &CONSOLE;
}
