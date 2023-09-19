#include "colors.h"
#include "netuno/array.h"
#include "netuno/nil/basic_block.h"
#include "netuno/nil/binary.h"
#include "netuno/nil/context.h"
#include "netuno/nil/type.h"
#include "pargument.h"
#include "pfunction.h"
#include "plist.h"
#include "pmodule.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/string.h>
#include <stdio.h>

NIL_MODULE *nilCreateModule(const char_t *name)
{
    NIL_MODULE *module = (NIL_MODULE *)ntMalloc(sizeof(NIL_MODULE));

    if (name)
        module->name = ntCopyCString(name);
    else
        module->name = NULL;
    module->sourceFileName = NULL;

    listInit(&module->list);

    return module;
}

void nilDestroyModule(NIL_MODULE *module)
{
    if (module->sourceFileName)
        ntUnrefString(module->sourceFileName);
    ntUnrefString(module->name);
    ntFree(module);
}

NT_STRING *nilGetSourceFileName(NIL_MODULE *module)
{
    return module->sourceFileName;
}

void nilSetSourceFileName(NIL_MODULE *module, const char_t *sourceFileName)
{
    if (module->sourceFileName)
        ntUnrefString(module->sourceFileName);
    module->sourceFileName = ntCopyCString(sourceFileName);
}

void nilSetModuleIdentifier(NIL_MODULE *module, const char_t *name)
{
    if (module->name)
        ntUnrefString(module->name);
    module->name = ntCopyCString(name);
}

NIL_FUNCTION *nilGetFunction(NIL_MODULE *module, const char_t *name)
{
    assert(module);
    assert(name);

    NT_STRING *strName = ntCopyCString(name);
    NIL_FUNCTION *result = NULL;

    for (size_t i = 0; i < module->list.count; ++i)
    {
        NIL_FUNCTION *function = module->functions[i];
        if (function->name == strName)
        {
            result = function;
            break;
        }
    }

    ntUnrefString(strName);
    return result;
}

NIL_FUNCTION *nilGetOrInsertFunction(NIL_MODULE *module, const char_t *name,
                                     NIL_TYPE *type)
{
    assert(module);
    assert(name);
    assert(type);

    NIL_CONTEXT *context = nilGetTypeContext(type);

    NIL_FUNCTION *function = nilGetFunction(module, name);
    if (function)
        return function;

    function = (NIL_FUNCTION *)ntMalloc(sizeof(NIL_FUNCTION));
    assert(function);

    function->name = ntCopyCString(name);
    NIL_FUNCTION_TYPE *functionType = function->functionType =
        (NIL_FUNCTION_TYPE *)type;

    const size_t argCount = function->argumentCount = functionType->paramCount;
    function->args = ntMalloc(sizeof(NIL_ARGUMENT) * argCount);

    for (size_t i = 0; i < argCount; ++i)
    {
        function->args[i].value.name = nilGetPrefixedId(context, U"arg");
        function->args[i].value.valueType = NIL_VALUE_TYPE_ARGUMENT;
        function->args[i].value.type = functionType->params[i];
        function->args[i].value.dbgLoc = NULL;

        function->args[i].function = function;
        function->args[i].argIndex = i;
    }

    listInit(&function->list);

    listAdd(&module->list, function);

    return function;
}

bool nilSerializeModule(NIL_MODULE *module, NT_ARRAY *array)
{
    assert(module);
    assert(array);

    ntArrayAddVarint(array, NIL_MODULE_BINARY, NULL);

    const char_t *name = ntStringChars(module->name, NULL);
    ntArrayAddString(array, name, NULL);

    const char_t *sourceFileName = ntStringChars(module->sourceFileName, NULL);
    ntArrayAddString(array, sourceFileName, NULL);

    bool success = true;
    for (size_t i = 0; i < module->list.count; ++i)
    {
        NIL_FUNCTION *function = module->functions[i];

        if (!nilSerializeFunction(function, array))
            success = false;
    }

    return success;
}

void nilPrintModule(NIL_MODULE *module)
{
    if (module->sourceFileName)
    {
        printf(CYN "source_filename" reset " =");
        ntPrintString(module->sourceFileName);
        printf("\n");
    }

    printf(CYN "module_id" reset " = ");
    ntPrintString(module->name);

    printf("\n\n");

    for (size_t i = 0; i < module->list.count; ++i)
    {
        NIL_FUNCTION *function = module->functions[i];

        nilPrintFunction(function);
    }
}
