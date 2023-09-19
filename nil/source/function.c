#include "colors.h"
#include "netuno/array.h"
#include "netuno/nil/basic_block.h"
#include "netuno/nil/binary.h"
#include "netuno/nil/value.h"
#include "netuno/string.h"
#include "pargument.h"
#include "pfunction.h"
#include <assert.h>
#include <stdio.h>

size_t nilGetParamCount(NIL_FUNCTION *function)
{
    assert(function);
    return function->argumentCount;
}

NIL_VALUE *nilGetParamValue(NIL_FUNCTION *function, size_t i)
{
    return (NIL_VALUE *)&function->args[i];
}

static bool serializeArgument(NIL_ARGUMENT *argument, NT_ARRAY *array)
{
    assert(argument);
    assert(array);

    // TODO
    assert(false);

    return true;
}

static bool serializeBlock(NIL_BASIC_BLOCK *block, NT_ARRAY *array)
{
    assert(block);
    assert(array);

    // TODO
    assert(false);

    return true;
}

bool nilSerializeFunction(NIL_FUNCTION *function, NT_ARRAY *array)
{
    assert(function);
    assert(array);

    // start function
    ntArrayAddVarint(array, NIL_FUNCTION_BINARY, NULL);

    // functio name
    const char_t *name = ntStringChars(function->name, NULL);
    ntArrayAddString(array, name, NULL);

    // function type
    nilSerializeType((NIL_TYPE *)function->functionType, array);

    // arguments
    ntArrayAddVarint(array, function->argumentCount, NULL);
    for (size_t i = 0; i < function->argumentCount; ++i)
    {
        NIL_ARGUMENT *argument = &function->args[i];
        serializeArgument(argument, array);
        // nilPrintValueName((NIL_VALUE *)argument);

        // if (i + 1 < function->argumentCount)
        // {
        //     printf(", ");
        // }
    }

    // blocks
    ntArrayAddVarint(array, function->list.count, NULL);
    for (size_t i = 0; i < function->list.count; ++i)
    {
        NIL_BASIC_BLOCK *block = function->blocks[i];
        serializeBlock(block, array);
    }

    return true;
}

void nilPrintFunction(NIL_FUNCTION *function)
{
    printf(RED "define " reset);
    nilPrintType(function->functionType->result);

    printf(" " YEL);
    ntPrintString(function->name);

    printf(reset "(");

    for (size_t i = 0; i < function->argumentCount; ++i)
    {
        NIL_ARGUMENT *argument = &function->args[i];
        nilPrintType(argument->value.type);
        printf(" ");
        nilPrintValueName((NIL_VALUE *)argument);

        if (i + 1 < function->argumentCount)
        {
            printf(", ");
        }
    }

    printf(") {\n");

    for (size_t i = 0; i < function->list.count; ++i)
    {
        NIL_BASIC_BLOCK *block = function->blocks[i];

        nilPrintBlock(block);
    }

    printf("}\n\n");
}
