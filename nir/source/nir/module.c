#include "colors.h"
#include "netuno/nir/basic_block.h"
#include "netuno/nir/context.h"
#include "netuno/nir/type.h"
#include "nir/pargument.h"
#include "nir/plist.h"
#include "pfunction.h"
#include "pmodule.h"
#include <assert.h>
#include <netuno/memory.h>
#include <netuno/string.h>
#include <stdio.h>

NIR_MODULE *nirCreateModule(const char_t *name)
{
    NIR_MODULE *module = (NIR_MODULE *)ntMalloc(sizeof(NIR_MODULE));

    if (name)
        module->name = ntCopyCString(name);
    else
        module->name = NULL;
    module->sourceFileName = NULL;

    listInit(&module->list);

    return module;
}

void nirDestroyModule(NIR_MODULE *module)
{
    if (module->sourceFileName)
        ntUnrefString(module->sourceFileName);
    ntUnrefString(module->name);
    ntFree(module);
}

NT_STRING *nirGetSourceFileName(NIR_MODULE *module)
{
    return module->sourceFileName;
}

void nirSetSourceFileName(NIR_MODULE *module, const char_t *sourceFileName)
{
    if (module->sourceFileName)
        ntUnrefString(module->sourceFileName);
    module->sourceFileName = ntCopyCString(sourceFileName);
}

void nirSetModuleIdentifier(NIR_MODULE *module, const char_t *name)
{
    if (module->name)
        ntUnrefString(module->name);
    module->name = ntCopyCString(name);
}

NIR_FUNCTION *nirGetFunction(NIR_MODULE *module, const char_t *name)
{
    assert(module);
    assert(name);

    NT_STRING *strName = ntCopyCString(name);
    NIR_FUNCTION *result = NULL;

    for (size_t i = 0; i < module->list.count; ++i)
    {
        NIR_FUNCTION *function = module->functions[i];
        if (function->name == strName)
        {
            result = function;
            break;
        }
    }

    ntUnrefString(strName);
    return result;
}

NIR_FUNCTION *nirGetOrInsertFunction(NIR_MODULE *module, const char_t *name,
                                     NIR_TYPE *type)
{
    assert(module);
    assert(name);
    assert(type);

    NIR_CONTEXT *context = nirGetTypeContext(type);

    NIR_FUNCTION *function = nirGetFunction(module, name);
    if (function)
        return function;

    function = (NIR_FUNCTION *)ntMalloc(sizeof(NIR_FUNCTION));
    assert(function);

    function->name = ntCopyCString(name);
    NIR_FUNCTION_TYPE *functionType = function->functionType =
        (NIR_FUNCTION_TYPE *)type;

    const size_t argCount = function->argumentCount = functionType->paramCount;
    function->args = ntMalloc(sizeof(NIR_ARGUMENT) * argCount);

    for (size_t i = 0; i < argCount; ++i)
    {
        function->args[i].value.name = nirGetPrefixedId(context, U"arg");
        function->args[i].value.valueType = NIR_VALUE_TYPE_ARGUMENT;
        function->args[i].value.type = functionType->params[i];
        function->args[i].value.dbgLoc = NULL;

        function->args[i].function = function;
        function->args[i].argIndex = i;
    }

    listInit(&function->list);

    listAdd(&module->list, function);

    return function;
}

void nirPrintModule(NIR_MODULE *module)
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
        NIR_FUNCTION *function = module->functions[i];

        nirPrintFunction(function);
    }
}
