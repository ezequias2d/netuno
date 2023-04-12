#include "colors.h"
#include "netuno/nir/basic_block.h"
#include "netuno/nir/value.h"
#include "netuno/string.h"
#include "nir/pargument.h"
#include "pfunction.h"
#include <assert.h>
#include <stdio.h>

size_t nirGetParamCount(NIR_FUNCTION *function)
{
    assert(function);
    return function->argumentCount;
}

NIR_VALUE *nirGetParamValue(NIR_FUNCTION *function, size_t i)
{
    return (NIR_VALUE *)&function->args[i];
}

void nirPrintFunction(NIR_FUNCTION *function)
{
    printf(RED "define " reset);
    nirPrintType(function->functionType->result);

    printf(" " YEL);
    ntPrintString(function->name);

    printf(reset "(");

    for (size_t i = 0; i < function->argumentCount; ++i)
    {
        NIR_ARGUMENT *argument = &function->args[i];
        nirPrintType(argument->value.type);
        printf(" ");
        nirPrintValueName((NIR_VALUE *)argument);

        if (i + 1 < function->argumentCount)
        {
            printf(", ");
        }
    }

    printf(") {\n");

    for (size_t i = 0; i < function->list.count; ++i)
    {
        NIR_BASIC_BLOCK *block = function->blocks[i];

        nirPrintBlock(block);
    }

    printf("}\n\n");
}
